#include "m_pd.h"

#include <math.h>

#define SENSOR_VALUE_COUNT 21
#define SENSOR_OUTPUT_COUNT 8
#define SENSOR_DISPLAY_LIMIT 2.0f
#define SENSOR_SOFT_KNEE 1.5f

typedef struct _sensor_calculate {
    t_object x_obj;
    t_outlet *x_data_out;
} t_sensor_calculate;

static t_class *sensor_calculate_class;

static const char *sensor_output_names[SENSOR_OUTPUT_COUNT] = {
    "acc-x",
    "acc-y",
    "acc-z",
    "acc",
    "gyro-x",
    "gyro-y",
    "gyro-z",
    "gyro"
};

static t_float soft_limit_positive(t_float value)
{
    if (value <= SENSOR_SOFT_KNEE)
        return value;

    return SENSOR_SOFT_KNEE
        + (SENSOR_DISPLAY_LIMIT - SENSOR_SOFT_KNEE)
        * (1.0f - (t_float)exp(
            -(value - SENSOR_SOFT_KNEE)
            / (SENSOR_DISPLAY_LIMIT - SENSOR_SOFT_KNEE)));
}

static t_float soft_limit_axis(t_float value)
{
    if (value < 0.0f)
        return -soft_limit_positive(-value);
    return soft_limit_positive(value);
}

static void sensor_calculate_values(
    t_sensor_calculate *x,
    int argc,
    t_atom *argv)
{
    t_float output[SENSOR_OUTPUT_COUNT];
    t_float sensor_id;
    t_float acc_x;
    t_float acc_y;
    t_float acc_z;
    t_float acc_magnitude;
    t_float gyro_x;
    t_float gyro_y;
    t_float gyro_z;
    t_float gyro_magnitude;
    int index;

    if (argc != SENSOR_VALUE_COUNT) {
        pd_error(x,
            "[sensor_calculate]: expected %d values, received %d",
            SENSOR_VALUE_COUNT,
            argc);
        return;
    }

    for (index = 0; index < argc; ++index) {
        if (argv[index].a_type != A_FLOAT) {
            pd_error(x, "[sensor_calculate]: all values must be numbers");
            return;
        }
    }

    /* Orm's final Clamps scaling. */
    acc_x = atom_getfloat(argv + 2) * (4.0f / 32768.0f);
    acc_y = atom_getfloat(argv + 3) * (4.0f / 32768.0f);
    acc_z = atom_getfloat(argv + 4) * (4.0f / 32768.0f);
    acc_magnitude = (t_float)sqrt(
        acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);

    gyro_x = atom_getfloat(argv + 5)
        * (2000.0f / (32768.0f * 360.0f));
    gyro_y = atom_getfloat(argv + 6)
        * (2000.0f / (32768.0f * 360.0f));
    gyro_z = atom_getfloat(argv + 7)
        * (2000.0f / (32768.0f * 360.0f));
    gyro_magnitude = (t_float)sqrt(
        gyro_x * gyro_x + gyro_y * gyro_y + gyro_z * gyro_z);

    sensor_id = atom_getfloat(argv);
    output[0] = soft_limit_axis(acc_x);
    output[1] = soft_limit_axis(acc_y);
    output[2] = soft_limit_axis(acc_z);
    output[3] = soft_limit_positive(acc_magnitude);
    output[4] = soft_limit_axis(gyro_x);
    output[5] = soft_limit_axis(gyro_y);
    output[6] = soft_limit_axis(gyro_z);
    output[7] = soft_limit_positive(gyro_magnitude);

    /* One named message per value: sensor <ID> <name> <value>. */
    for (index = 0; index < SENSOR_OUTPUT_COUNT; ++index) {
        t_atom message[3];

        SETFLOAT(&message[0], sensor_id);
        SETSYMBOL(&message[1], gensym(sensor_output_names[index]));
        SETFLOAT(&message[2], output[index]);
        outlet_anything(x->x_data_out, gensym("sensor"), 3, message);
    }
}

static void sensor_calculate_raw(
    t_sensor_calculate *x,
    t_symbol *selector,
    int argc,
    t_atom *argv)
{
    (void)selector;
    sensor_calculate_values(x, argc, argv);
}

static void sensor_calculate_list(
    t_sensor_calculate *x,
    t_symbol *selector,
    int argc,
    t_atom *argv)
{
    (void)selector;
    sensor_calculate_values(x, argc, argv);
}

static void *sensor_calculate_new(void)
{
    t_sensor_calculate *x =
        (t_sensor_calculate *)pd_new(sensor_calculate_class);

    x->x_data_out = outlet_new(&x->x_obj, &s_anything);
    return x;
}

void sensor_calculate_setup(void)
{
    sensor_calculate_class = class_new(
        gensym("sensor_calculate"),
        (t_newmethod)sensor_calculate_new,
        0,
        sizeof(t_sensor_calculate),
        CLASS_DEFAULT,
        0);

    class_addmethod(
        sensor_calculate_class,
        (t_method)sensor_calculate_raw,
        gensym("sensor_raw"),
        A_GIMME,
        0);
    class_addlist(
        sensor_calculate_class,
        (t_method)sensor_calculate_list);
}
