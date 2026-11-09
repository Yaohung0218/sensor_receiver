#include "m_pd.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#define SENSOR_COMMAND_BYTE_COUNT 9
#define SENSOR_ACCEL_LSB_PER_G 8192.0f

#define SENSOR_FRAME_RESET 0xffu
#define SENSOR_FRAME_COMMAND 0xccu
#define SENSOR_FRAME_PARAMETER 0xfeu

#define SENSOR_PARAMETER_THRESHOLD 0x05u
#define SENSOR_PARAMETER_FREQUENCY 0x06u

typedef struct _sensor_command {
    t_object x_obj;
    t_outlet *x_bytes_out;
} t_sensor_command;

static t_class *sensor_command_class;

static int sensor_command_valid_id(
    t_sensor_command *x,
    t_floatarg id_value,
    uint8_t *id)
{
    int numeric_id = (int)id_value;

    if (id_value != (t_float)numeric_id
        || numeric_id < 1
        || numeric_id > 127) {
        pd_error(x,
            "[sensor_command]: sensor ID must be an integer from 1 to 127");
        return 0;
    }

    *id = (uint8_t)numeric_id;
    return 1;
}

static void sensor_command_output_parameter(
    t_sensor_command *x,
    uint8_t sensor_id,
    uint8_t parameter_id,
    float value)
{
    uint32_t bits;
    uint8_t bytes[SENSOR_COMMAND_BYTE_COUNT];
    t_atom output[SENSOR_COMMAND_BYTE_COUNT];
    int index;

    memcpy(&bits, &value, sizeof(bits));

    bytes[0] = SENSOR_FRAME_RESET;
    bytes[1] = SENSOR_FRAME_COMMAND;
    bytes[2] = sensor_id;
    bytes[3] = SENSOR_FRAME_PARAMETER;
    bytes[4] = parameter_id;
    bytes[5] = (uint8_t)((bits >> 24) & 0xffu);
    bytes[6] = (uint8_t)((bits >> 16) & 0xffu);
    bytes[7] = (uint8_t)((bits >> 8) & 0xffu);
    bytes[8] = (uint8_t)(bits & 0xffu);

    for (index = 0; index < SENSOR_COMMAND_BYTE_COUNT; ++index)
        SETFLOAT(&output[index], (t_float)bytes[index]);

    outlet_list(x->x_bytes_out, &s_list,
        SENSOR_COMMAND_BYTE_COUNT, output);
}

static void sensor_command_freq(
    t_sensor_command *x,
    t_floatarg id_value,
    t_floatarg frequency)
{
    uint8_t sensor_id;

    if (!sensor_command_valid_id(x, id_value, &sensor_id))
        return;
    if (!isfinite((double)frequency) || frequency < 0.0f) {
        pd_error(x,
            "[sensor_command]: frequency must be a finite number >= 0");
        return;
    }

    sensor_command_output_parameter(x, sensor_id,
        SENSOR_PARAMETER_FREQUENCY, (float)frequency);
}

static void sensor_command_threshold(
    t_sensor_command *x,
    t_floatarg id_value,
    t_floatarg threshold_g)
{
    uint8_t sensor_id;
    float threshold_lsb;

    if (!sensor_command_valid_id(x, id_value, &sensor_id))
        return;
    if (!isfinite((double)threshold_g) || threshold_g < 0.0f) {
        pd_error(x,
            "[sensor_command]: threshold must be a finite number >= 0");
        return;
    }

    /* The UI value is in g. The firmware parameter is raw accelerometer
       LSB, matching Orm's CLAMPS conversion. */
    threshold_lsb = roundf((float)threshold_g * SENSOR_ACCEL_LSB_PER_G);
    sensor_command_output_parameter(x, sensor_id,
        SENSOR_PARAMETER_THRESHOLD, threshold_lsb);
}

static void *sensor_command_new(void)
{
    t_sensor_command *x =
        (t_sensor_command *)pd_new(sensor_command_class);

    x->x_bytes_out = outlet_new(&x->x_obj, &s_list);
    return x;
}

void sensor_command_setup(void)
{
    sensor_command_class = class_new(
        gensym("sensor_command"),
        (t_newmethod)sensor_command_new,
        0,
        sizeof(t_sensor_command),
        CLASS_DEFAULT,
        0);

    class_addmethod(sensor_command_class,
        (t_method)sensor_command_freq,
        gensym("freq"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(sensor_command_class,
        (t_method)sensor_command_threshold,
        gensym("threshold"), A_FLOAT, A_FLOAT, 0);
}
