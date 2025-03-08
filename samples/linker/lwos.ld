OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)


SECTIONS
{
 . = 0x40000000;
 .text ALIGN(0x1000):
 {
  __text_start = .;

  *(.init.text)
  *(.text)
  *(.text*)

  . = ALIGN(0x1000);
  __text_end = .;
 }

 .rodata ALIGN(0x1000):
 {
  __rodata_start = .;

  *(.rodata)
  *(.rodata*)

  . = ALIGN(0x1000);
  __rodata_end = .;
 }

 .bss ALIGN(0x1000):
 {
  __bss_start = .;

  *(.bss)
  *(.bss*)

  . = ALIGN(0x1000);
  __bss_end = .;
 }

 .data ALIGN(0x1000):
 {
  __data_start = .;

  *(.data)
  *(.data*)

  . = ALIGN(0x1000);
  __data_end = .;
 }

 .global_data ALIGN(0x1000):
 {
  __global_data_start = .;

  *(.global_data)
  *(.global_data*)

  . = ALIGN(0x1000);
  __global_data_end = .;
 }

 .shell ALIGN(0x1000):
 {
  __shell_cmd_start = .;

  __shell_root_cmd_start = .;
  *(.shell.root.cmd*)
  __shell_root_cmd_end = .;
  . += (8 * 128);
  __shell_sub_cmd_start = .;
  *(.shell.sub.cmd*)
  __shell_sub_cmd_end = .;

  . = ALIGN(0x1000);
  __shell_cmd_end = .;
 }

 .page_table_pool ALIGN(0x1000):
 {
  __page_table_pool_start = .;
  . += 0x200000;
  __page_table_pool_end = .;
 }

 .kernel_stack ALIGN(0x1000):
 {
  __kernel_stack_start = .;
  . += (8192 * 2);
  . = ALIGN(0x1000);
  __kernel_stack_end = .;
 }

 .kernel_heap ALIGN(0x1000):
 {
  __kernel_heap_start = .;
  . += (4000000);
  . = ALIGN(0x1000);
  __kernel_heap_end = .;
 }

 .interrupt_stack ALIGN(0x1000):
 {
  __interrupt_stack_start = .;
  . += (8192 * 2);
  . = ALIGN(0x1000);
  __interrupt_stack_end = .;
 }






}
