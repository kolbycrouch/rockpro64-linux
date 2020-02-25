// SPDX-License-Identifier: GPL-2.0-only
/*
 * STMicroelectronics sensors i2c library driver
 *
 * Copyright 2012-2013 STMicroelectronics Inc.
 *
 * Denis Ciocca <denis.ciocca@st.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/iio/iio.h>
#include <linux/of_device.h>
#include <linux/acpi.h>
#include <linux/regmap.h>

#include <linux/iio/common/st_sensors_i2c.h>


#define ST_SENSORS_I2C_MULTIREAD	0x80

static const struct regmap_config st_sensors_i2c_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static const struct regmap_config st_sensors_i2c_regmap_multiread_bit_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.read_flag_mask = ST_SENSORS_I2C_MULTIREAD,
};

/*
 * st_sensors_i2c_configure() - configure I2C interface
 * @indio_dev: IIO device reference.
 * @client: i2c client reference.
 *
 * Return: 0 on success, else a negative error code.
 */
int st_sensors_i2c_configure(struct iio_dev *indio_dev,
			     struct i2c_client *client)
{
	struct st_sensor_data *sdata = iio_priv(indio_dev);
	const struct regmap_config *config;

	if (sdata->sensor_settings->multi_read_bit)
		config = &st_sensors_i2c_regmap_multiread_bit_config;
	else
		config = &st_sensors_i2c_regmap_config;

	sdata->regmap = devm_regmap_init_i2c(client, config);
	if (IS_ERR(sdata->regmap)) {
		dev_err(&client->dev, "Failed to register i2c regmap (%d)\n",
			(int)PTR_ERR(sdata->regmap));
		return PTR_ERR(sdata->regmap);
	}

	i2c_set_clientdata(client, indio_dev);

	indio_dev->dev.parent = &client->dev;
	indio_dev->name = client->name;

	sdata->dev = &client->dev;
	sdata->irq = client->irq;

	return 0;
}
EXPORT_SYMBOL(st_sensors_i2c_configure);

#ifdef CONFIG_ACPI
int st_sensors_match_acpi_device(struct device *dev)
{
	const struct acpi_device_id *acpi_id;
	kernel_ulong_t driver_data = 0;

	if (ACPI_HANDLE(dev)) {
		acpi_id = acpi_match_device(dev->driver->acpi_match_table, dev);
		if (!acpi_id) {
			dev_err(dev, "No driver data\n");
			return -EINVAL;
		}
		driver_data = acpi_id->driver_data;
	}
	return driver_data;
}
EXPORT_SYMBOL(st_sensors_match_acpi_device);
#endif

MODULE_AUTHOR("Denis Ciocca <denis.ciocca@st.com>");
MODULE_DESCRIPTION("STMicroelectronics ST-sensors i2c driver");
MODULE_LICENSE("GPL v2");
