#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x65ac75d6, "module_layout" },
	{ 0x24482750, "misc_deregister" },
	{ 0x4a181c26, "sysfs_remove_group" },
	{ 0xfe990052, "gpio_free" },
	{ 0x9cbecbcd, "input_register_device" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xb80f56a6, "input_free_device" },
	{ 0xbc601ad6, "script_get_item" },
	{ 0xd15a98df, "input_allocate_device" },
	{ 0xc09d0994, "sysfs_create_group" },
	{ 0x8af1e6c, "misc_register" },
	{ 0x91715312, "sprintf" },
	{ 0x27e1a049, "printk" },
	{ 0x20000329, "simple_strtoul" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "07A7712F06AF805C0F8C807");
