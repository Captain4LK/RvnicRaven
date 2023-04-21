# Documentation overview

## Types {#types}

[RvR_fix16](fix16)

[RvR_fix24](fix24)

[RvR_rand_pcg](rand_pcg)

[RvR_rand_well](rand_well)

[RvR_rand_game](rand_game)

[RvR_rand_xor](rand_xor)

[RvR_key](key)

[RvR_gamepad_button](gamepad_button)

[RvR_ppp_ccontext](ppp_ccontext)

[RvR_ppp_dcontext](ppp_dcontext)

[RvR_color](color)

[RvR_rw_type](rw_type)

[RvR_rw_endian](rw_endian)

[RvR_rw](rw)

[RvR_rw_usr_init](rw_usr_init)

[RvR_rw_usr_close](rw_usr_close)

[RvR_rw_usr_flush](rw_usr_flush)

[RvR_rw_usr_seek](rw_usr_seek)

[RvR_rw_usr_tell](rw_usr_tell)

[RvR_rw_usr_eof](rw_usr_eof)

[RvR_rw_usr_read](rw_usr_read)

[RvR_rw_usr_write](write)

[RvR_texture](texture)

## Constants {#constants}

[RVR_HASH_FNV32_INIT](/rvr/rvr/hash_fnv32_init)

[RVR_HASH_FNV64_INIT](/rvr/rvr/hash_fnv64_init)

[RVR_XRES_MAX](xres_max)

[RVR_YRES_MAX](yres_max)

## Macros {#macros}

[RvR_max](/rvr/rvr/max)

[RvR_min](/rvr/rvr/min)

[RvR_clamp](/rvr/rvr/clamp)

[RvR_abs](/rvr/rvr/abs)

[RvR_non_zero](/rvr/rvr/non_zero)

[RvR_sign_equal](/rvr/rvr/sign_equal)

[RvR_array_push](array_push)

[RvR_array_length](array_length)

[RvR_array_length_set](array_length_set)

[RvR_array_free](array_free)

[RvR_log_line](log_line)

[RvR_error_check](error_check)

## Functions {#functions}

[RvR_fix16_mul](/rvr/rvr/fix16_mul)

[RvR_fix16_div](/rvr/rvr/fix16_div)

[RvR_fix16_cos](/rvr/rvr/fix16_cos)

[RvR_fix16_sin](/rvr/rvr/fix16_sin)

[RvR_fix16_tan](/rvr/rvr/fix16_tan)

[RvR_fix16_sqrt](/rvr/rvr/fix16_sqrt)

[RvR_fix24_mul](/rvr/rvr/fix24_mul)

[RvR_fix24_div](/rvr/rvr/fix24_div)

[RvR_rand_pcg_seed](/rvr/rvr/rand_pcg_seed)

[RvR_rand_pcg_next](/rvr/rvr/rand_pcg_next)

[RvR_rand_pcg_next_range](/rvr/rvr/rand_pcg_next_range)

[RvR_rand_well_seed](/rvr/rvr/rand_well_seed)

[RvR_rand_well_next](/rvr/rvr/rand_well_next)

[RvR_rand_well_next_range](/rvr/rvr/rand_well_next_range)

[RvR_rand_game_seed](/rvr/rvr/rand_game_seed)

[RvR_rand_game_next](/rvr/rvr/rand_game_next)

[RvR_rand_game_next_range](/rvr/rvr/rand_game_next_range)

[RvR_rand_xor_seed](/rvr/rvr/rand_xor_seed)

[RvR_rand_xor_next](/rvr/rvr/rand_xor_next)

[RvR_rand_xor_next_range](/rvr/rvr/rand_xor_next_range)

[RvR_fnv32a](/rvr/rvr/fnv32a)

[RvR_fnv32a_str](/rvr/rvr/fnv32a_str)

[RvR_fnv32a_buf](/rvr/rvr/fnv32a_buf)

[RvR_fnv64a](/rvr/rvr/fnv64a)

[RvR_fnv64a_str](/rvr/rvr/fnv64a_str)

[RvR_fnv64a_buf](/rvr/rvr/fnv64a_buf)

[RvR_init](init)

[RvR_quit](quit)

[RvR_running](running)

[RvR_update](update)

[RvR_frametime](frametime)

[RvR_frametime_average](frametime_average)

[RvR_render_present](render_present)

[RvR_framebuffer](framebuffer)

[RvR_frame](frame)

[RvR_xres](xres)

[RvR_yres](yres)

[RvR_fps](fps)

[RvR_key_down](key_down)

[RvR_key_pressed](key_pressed)

[RvR_key_released](key_released)

[RvR_key_repeat](key_repeat)

[RvR_gamepad_down](gamepad_down)

[RvR_gamepad_pressed](gamepad_pressed)

[RvR_gamepad_released](gamepad_released)

[RvR_mouse_relative_pos](mouse_relative_pos)

[RvR_mouse_pos](mouse_pos)

[RvR_mouse_set_pos](mouse_set_pos)

[RvR_mouse_relative](mouse_relative)

[RvR_mouse_show](mouse_show)

[RvR_mouse_wheel_scroll](mouse_wheel_scroll)

[RvR_text_input_start](text_input_start)

[RvR_text_input_end](text_input_end)

[RvR_clip_line](clip_line)

[RvR_crush_compress](crush_compress)

[RvR_crush_decompress](crush_decompress)

[RvR_ppp_compress_init](ppp_compress_init)

[RvR_ppp_compress_push](ppp_compress_push)

[RvR_ppp_compress_flush](ppp_compress_flush)

[RvR_ppp_decompress_init](ppp_decompress_init)

[RvR_ppp_decompress_pop](ppp_decompress_pop)

[RvR_log](log)

[RvR_malloc_init](malloc_init)

[RvR_malloc](malloc)

[RvR_malloc_inane](malloc_inane)

[RvR_free](free)

[RvR_realloc](realloc)

[RvR_realloc_inane](realloc_inane)

[RvR_malloc_report](malloc_report)

[RvR_malloc_base](malloc_base)

[RvR_pak_add](pak_add)

[RvR_pak_create_from_csv](pak_create_from_csv)

[RvR_pak_flush](pak_flush)

[RvR_lump_add](lump_add)

[RvR_lump_get](lump_get)

[RvR_lump_get_path](lump_get_path)

[RvR_lump_exists](lump_exists)

[RvR_palette_load](palette_load)

[RvR_palette](palette)

[RvR_shade_table](shade_table)

[RvR_blend](blend)

[RvR_render_clear](render_clear)

[RvR_render_texture](render_texture)

[RvR_render_texture2](render_texture2)

[RvR_render_rectangle](render_rectangle)

[RvR_render_rectangle_fill](render_rectangle_fill)

[RvR_render_circle](render_circle)

[RvR_render_font_set](render_font_set)

[RvR_render_string](render_string)

[RvR_render_line](render_line)

[RvR_render_vertical_line](render_vertical_line)

[RvR_render_horizontal_line](render_horizontal_line)

[RvR_rw_init_file](rw_init_file)

[RvR_rw_init_path](rw_init_path)

[RvR_rw_init_mem](rw_init_mem)

[RvR_rw_init_dyn_mem](rw_init_dyn_mem)

[RvR_rw_init_const_mem](rw_init_const_mem)

[RvR_rw_init_usr](rw_init_usr)

[RvR_rw_valid](rw_valid)

[RvR_rw_endian_set](rw_endian_set)

[RvR_rw_close](rw_close)

[RvR_rw_flush](rw_flush)

[RvR_rw_seek](rw_seek)

[RvR_rw_tell](rw_tell)

[RvR_rw_eof](rw_eof)

[RvR_rw_read](rw_read)

[RvR_rw_write](rw_write)

[RvR_rw_printf](rw_printf)

[RvR_rw_write_u8](rw_write_u8)

[RvR_rw_write_u16](rw_write_u16)

[RvR_rw_write_u32](rw_write_u32)

[RvR_rw_write_u64](rw_write_u64)

[RvR_rw_read_u8](rw_read_u8)

[RvR_rw_read_u16](rw_read_u16)

[RvR_rw_read_u32](rw_read_u32)

[RvR_rw_read_u64](rw_read_u64)

[RvR_texture_get](texture_get)

[RvR_texture_create](texture_create)

[RvR_texture_create_free](texture_create_free)
