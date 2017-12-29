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
	{ 0x8af1e6c, "misc_register" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0xbc601ad6, "script_get_item" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0x27e1a049, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "8CF3F620F53B5CE212A33F0");
