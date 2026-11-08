#include "m_pd.h"

#include <stdint.h>
#include <string.h>

#define SENSOR_PAYLOAD_BYTES 51

typedef struct _sensor_decode {
    t_object x_obj;
    t_outlet *x_data_out;
    t_outlet *x_status_out;
    uint8_t frame[SENSOR_PAYLOAD_BYTES];
    size_t frame_len;
    unsigned long valid_frames;
    unsigned long invalid_frames;
    unsigned long invalid_bytes;
} t_sensor_decode;

static t_class *sensor_decode_class;

static void atom_push_float(t_atom *atoms, int *count, t_float value)
{
    SETFLOAT(&atoms[*count], value);
    *count += 1;
}

static int16_t signed_16(uint16_t value)
{
    if (value >= 32768u)
        return (int16_t)((int32_t)value - 65536);
    return (int16_t)value;
}

static void decode_triplet(const uint8_t *bytes, int16_t result[3])
{
    uint16_t z = (uint16_t)(((uint16_t)bytes[0] << 9)
        | ((uint16_t)bytes[1] << 2)
        | ((uint16_t)(bytes[2] & 0x60u) >> 5));
    uint16_t y = (uint16_t)(((uint16_t)(bytes[2] & 0x1fu) << 11)
        | ((uint16_t)bytes[3] << 4)
        | ((uint16_t)(bytes[4] & 0x78u) >> 3));
    uint16_t x = (uint16_t)(((uint16_t)(bytes[4] & 0x07u) << 13)
        | ((uint16_t)bytes[5] << 6)
        | ((uint16_t)(bytes[6] & 0x7eu) >> 1));

    result[0] = signed_16(x);
    result[1] = signed_16(y);
    result[2] = signed_16(z);
}

static int frame_is_valid(const uint8_t *frame)
{
    int index;

    if (frame[0] < 1 || frame[0] > 127 || frame[1] > 127)
        return 0;

    for (index = 2; index < SENSOR_PAYLOAD_BYTES; ++index) {
        if (frame[index] > 127)
            return 0;
    }
    return 1;
}

static void output_frame(t_sensor_decode *x, const uint8_t *frame)
{
    int16_t accel[3];
    int16_t gyro[3];
    int16_t mag[3];
    int16_t quat_wx[3];
    int16_t quat_yz[3];
    int16_t euler[3];
    int16_t velocity[3];
    t_atom values[21];
    int count = 0;
    int index;

    decode_triplet(frame + 2, accel);
    decode_triplet(frame + 9, gyro);
    decode_triplet(frame + 16, mag);
    decode_triplet(frame + 23, quat_wx);
    decode_triplet(frame + 30, quat_yz);
    decode_triplet(frame + 37, euler);
    decode_triplet(frame + 44, velocity);

    atom_push_float(values, &count, (t_float)frame[0]);
    atom_push_float(values, &count, (t_float)frame[1]);

    for (index = 0; index < 3; ++index)
        atom_push_float(values, &count, (t_float)accel[index]);
    for (index = 0; index < 3; ++index)
        atom_push_float(values, &count, (t_float)gyro[index]);
    for (index = 0; index < 3; ++index)
        atom_push_float(values, &count, (t_float)mag[index]);

    atom_push_float(values, &count, (t_float)quat_wx[0]);
    atom_push_float(values, &count, (t_float)quat_wx[1]);
    atom_push_float(values, &count, (t_float)quat_yz[0]);
    atom_push_float(values, &count, (t_float)quat_yz[1]);

    for (index = 0; index < 3; ++index)
        atom_push_float(values, &count, (t_float)euler[index]);
    for (index = 0; index < 3; ++index)
        atom_push_float(values, &count, (t_float)velocity[index]);

    outlet_anything(x->x_data_out, gensym("sensor_raw"), count, values);
}

static void sensor_decode_float(t_sensor_decode *x, t_floatarg value)
{
    int byte = (int)value;

    if (value != (t_float)byte || byte < 0 || byte > 255) {
        x->invalid_bytes += 1;
        return;
    }

    if (byte == 255) {
        if (x->frame_len == SENSOR_PAYLOAD_BYTES
            && frame_is_valid(x->frame)) {
            output_frame(x, x->frame);
            x->valid_frames += 1;
        } else {
            x->invalid_frames += 1;
        }
        x->frame_len = 0;
        return;
    }

    /* Keep only the newest 51 bytes. Startup text and MAC announcements
       are therefore discarded automatically at the next 255 delimiter. */
    if (x->frame_len < SENSOR_PAYLOAD_BYTES) {
        x->frame[x->frame_len] = (uint8_t)byte;
        x->frame_len += 1;
    } else {
        memmove(x->frame, x->frame + 1, SENSOR_PAYLOAD_BYTES - 1);
        x->frame[SENSOR_PAYLOAD_BYTES - 1] = (uint8_t)byte;
    }
}

static void sensor_decode_reset(t_sensor_decode *x)
{
    x->frame_len = 0;
    x->valid_frames = 0;
    x->invalid_frames = 0;
    x->invalid_bytes = 0;
}

static void sensor_decode_bang(t_sensor_decode *x)
{
    t_atom stats[3];

    SETFLOAT(&stats[0], (t_float)x->valid_frames);
    SETFLOAT(&stats[1], (t_float)x->invalid_frames);
    SETFLOAT(&stats[2], (t_float)x->invalid_bytes);
    outlet_anything(x->x_status_out, gensym("stats"), 3, stats);
}

static void *sensor_decode_new(void)
{
    t_sensor_decode *x = (t_sensor_decode *)pd_new(sensor_decode_class);

    x->x_data_out = outlet_new(&x->x_obj, &s_anything);
    x->x_status_out = outlet_new(&x->x_obj, &s_anything);
    sensor_decode_reset(x);
    return x;
}

void sensor_decode_setup(void)
{
    sensor_decode_class = class_new(
        gensym("sensor_decode"),
        (t_newmethod)sensor_decode_new,
        0,
        sizeof(t_sensor_decode),
        CLASS_DEFAULT,
        0
    );

    class_addfloat(sensor_decode_class, sensor_decode_float);
    class_addbang(sensor_decode_class, sensor_decode_bang);
    class_addmethod(
        sensor_decode_class,
        (t_method)sensor_decode_reset,
        gensym("reset"),
        0
    );
}
