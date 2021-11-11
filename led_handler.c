#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Javier Albors Sanchez");

static unsigned int gpio_led1 = 16;
static unsigned int gpio_led2 = 20;

static unsigned int gpio_bt1 = 13;
static unsigned int gpio_bt2 = 19;
static unsigned int gpio_bt3 = 26;
static unsigned int gpio_bt4 = 21;

static unsigned int irq_bt1;
static unsigned int irq_bt2;
static unsigned int irq_bt3;
static unsigned int irq_bt4;

static unsigned int bt1_clicks = 0;
static unsigned int bt2_clicks = 0;
static unsigned int bt3_clicks = 0;
static unsigned int bt4_clicks = 0;

static irq_handler_t bt1_handler(unsigned int irq, void *device, struct pt_regs *regs);
static irq_handler_t bt2_handler(unsigned int irq, void *device, struct pt_regs *regs);
static irq_handler_t bt3_handler(unsigned int irq, void *device, struct pt_regs *regs);
static irq_handler_t bt4_handler(unsigned int irq, void *device, struct pt_regs *regs);
static void log_message(char *message);

static int __init i_module(void) {

	int err = 0;

	log_message("Starting module");

	if (!gpio_is_valid(gpio_led1) || !gpio_is_valid(gpio_led2)
	|| !gpio_is_valid(gpio_bt1) || !gpio_is_valid(gpio_bt2) || !gpio_is_valid(gpio_bt3) || !gpio_is_valid(gpio_bt4)) {

		log_message("ERROR - Invalid GPIO(s)");
		return -ENODEV;
	}

	gpio_request(gpio_led1, "sysfs");
	gpio_direction_output(gpio_led1, 0);
	gpio_export(gpio_led1, false);

	gpio_request(gpio_led2, "sysfs");
	gpio_direction_output(gpio_led2, 0);
	gpio_export(gpio_led2, false);

	gpio_request(gpio_bt1, "sysfs");
	gpio_direction_input(gpio_bt1);
	gpio_set_debounce(gpio_bt1, 200);
	gpio_export(gpio_bt1, false);

	gpio_request(gpio_bt2, "sysfs");
	gpio_direction_input(gpio_bt2);
	gpio_set_debounce(gpio_bt2, 200);
	gpio_export(gpio_bt2, false);

	gpio_request(gpio_bt3, "sysfs");
	gpio_direction_input(gpio_bt3);
	gpio_set_debounce(gpio_bt3, 200);
	gpio_export(gpio_bt3, false);

	gpio_request(gpio_bt4, "sysfs");
	gpio_direction_input(gpio_bt4);
	gpio_set_debounce(gpio_bt4, 200);
	gpio_export(gpio_bt4, false);

	log_message("GPIO requests OK");

	irq_bt1 = gpio_to_irq(gpio_bt1);
	irq_bt2 = gpio_to_irq(gpio_bt2);
	irq_bt3 = gpio_to_irq(gpio_bt3);
	irq_bt4 = gpio_to_irq(gpio_bt4);

	err = request_irq(irq_bt1, (irq_handler_t) bt1_handler, IRQF_TRIGGER_RISING, "bt1_handler", NULL);
	if (err) return err;
	err = request_irq(irq_bt2, (irq_handler_t) bt2_handler, IRQF_TRIGGER_RISING, "bt2_handler", NULL);
	if (err) return err;
	err = request_irq(irq_bt3, (irq_handler_t) bt3_handler, IRQF_TRIGGER_RISING, "bt3_handler", NULL);
	if (err) return err;
	err = request_irq(irq_bt4, (irq_handler_t) bt4_handler, IRQF_TRIGGER_RISING, "bt4_handler", NULL);

	if(!err) log_message("IRQ handler requests OK");

	return err;
}

static void __exit e_module(void) {
	char message[100];

	log_message("Exiting module...");

	sprintf(message, "Button 1 was pressed %d times", bt1_clicks);
	log_message(message);

	sprintf(message, "Button 2 was pressed %d times", bt2_clicks);
	log_message(message);

	sprintf(message, "Button 3 was pressed %d times", bt3_clicks);
	log_message(message);

	sprintf(message, "Button 4 was pressed %d times", bt4_clicks);
	log_message(message);

	gpio_set_value(gpio_led1, 0);
	gpio_set_value(gpio_led2, 0);

	gpio_unexport(gpio_led1);
	gpio_unexport(gpio_led2);
	gpio_free(gpio_led1);
	gpio_free(gpio_led2);

	free_irq(irq_bt1, NULL);
	free_irq(irq_bt2, NULL);
	free_irq(irq_bt3, NULL);
	free_irq(irq_bt4, NULL);

	gpio_unexport(gpio_bt1);
	gpio_unexport(gpio_bt2);
	gpio_unexport(gpio_bt3);
	gpio_unexport(gpio_bt4);

	gpio_free(gpio_bt1);
	gpio_free(gpio_bt2);
	gpio_free(gpio_bt3);
	gpio_free(gpio_bt4);

	log_message("Module removed");
}

module_init(i_module);
module_exit(e_module);

static irq_handler_t bt1_handler(unsigned int irq, void *device, struct pt_regs *regs) {
	char *argv[] = {"/bin/bash", "-c", "/home/pi/led-handler-LKM/B1.sh", NULL};
	char message[100];

	gpio_set_value(gpio_led1, 1);

	call_usermodehelper(argv[0], argv, NULL, UMH_NO_WAIT);

	sprintf(message, "Button 1 pressed (pressed %d times) - LED 1: ON", ++bt1_clicks);
	log_message(message);

	return (irq_handler_t) IRQ_HANDLED;
}

static irq_handler_t bt2_handler(unsigned int irq, void *device, struct pt_regs *regs) {
	char *argv[] = {"/bin/bash", "-c", "/home/pi/led-handler-LKM/B2.sh", NULL};
	char message[100];

	gpio_set_value(gpio_led1, 0);

	call_usermodehelper(argv[0], argv, NULL, UMH_NO_WAIT);

	sprintf(message, "Button 2 pressed (pressed %d times) - LED 1: OFF", ++bt2_clicks);
	log_message(message);

	return (irq_handler_t) IRQ_HANDLED;
}

static irq_handler_t bt3_handler(unsigned int irq, void *device, struct pt_regs *regs) {
	char *argv[] = {"/bin/bash", "-c", "/home/pi/led-handler-LKM/B3.sh", NULL};
	char message[100];

	gpio_set_value(gpio_led2, 0);

	call_usermodehelper(argv[0], argv, NULL, UMH_NO_WAIT);

	sprintf(message, "Button 3 pressed (pressed %d times) - LED 2: OFF", ++bt3_clicks);
	log_message(message);

	return (irq_handler_t) IRQ_HANDLED;
}

static irq_handler_t bt4_handler(unsigned int irq, void *device, struct pt_regs *regs) {
	char *argv[] = {"/bin/bash", "-c", "/home/pi/led-handler-LKM/B4.sh", NULL};
	char message[100];

	gpio_set_value(gpio_led2, 1);

	call_usermodehelper(argv[0], argv, NULL, UMH_NO_WAIT);

	sprintf(message, "Button 4 pressed (pressed %d times) - LED 2: ON", ++bt4_clicks);
	log_message(message);

	return (irq_handler_t) IRQ_HANDLED;
}

static void log_message(char *message) {
	printk("led_handler: %s\n", message);
}
