
SECTIONS
{
    . = ALIGN(16);
    __app_init_start__ = .;
    .app_init_sec : { *(.app_init_sec) }
    __app_init_end__ = .;
}
INSERT AFTER .rodata
