#include <stdlib.h>
#include <string.h>
#include <ecl/ecl.h>

#include "machine.h"

static tic_machine* getMachine(void)
{
	return (tic_machine*)ecl_to_pointer(ecl_symbol_value(ecl_read_from_cstring("**machine**")));
}

static const char* asText(cl_object obj)
{
	return ecl_base_string_pointer_safe(si_coerce_to_base_string(cl_princ_to_string(obj)));
}

static s32 asInt(cl_object obj)
{
	return ecl_to_int32_t(ecl_truncate1(obj));
}

static float asFloat(cl_object obj)
{
	return ecl_to_float(obj);
}

static s32 asColor(cl_object obj)
{
	return asInt(obj) % TIC_PALETTE_SIZE;
}

static s32 asX(cl_object obj)
{
	return asInt(obj);
}

static s32 asY(cl_object obj)
{
	return asInt(obj);
}

static bool asBool(cl_object obj)
{
	return !Null(obj);
}

static cl_object asObject(cl_object obj)
{
	return obj;
}

#define POPARG(fn, def) (nargs > 0 ? nargs--, fn(ecl_va_arg(ap)) : (def))

#define RETINT(x) result = ecl_make_int32_t((x))

#define RETBOOL(x) result = (x) ? ECL_T : ECL_NIL

#define APIFUNC(name)						\
	static cl_object clapi_ ## name(cl_narg nargs, ...)	\
	{							\
	tic_machine* machine = getMachine();			\
	tic_mem* memory = (tic_mem*)machine;			\
	ecl_va_list ap;						\
	cl_object result = ECL_NIL;				\
	ecl_va_start(ap, nargs, nargs, 0);			\
	(void)memory;

#define APIFUNC_END 				\
	ecl_va_end(ap);				\
	ecl_return1(ecl_process_env(), result); \
	}

APIFUNC(print)
{
	const char* text = POPARG(asText, "");
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 color = POPARG(asColor, TIC_PALETTE_SIZE - 1);
	bool fixed = POPARG(asBool, false);
	s32 scale = POPARG(asInt, 1);
	s32 size = 0;
	if (scale > 0)
	{
		size = memory->api.text_ex(memory, text ? text : "nil", x, y, color, fixed, scale);
	}
	RETINT(size);
}
APIFUNC_END

APIFUNC(cls)
{
	memory->api.clear(memory, POPARG(asColor, 0));
}
APIFUNC_END

APIFUNC(pix)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 color = POPARG(asColor, -1);
	if (color == -1)
	{
		RETINT(memory->api.get_pixel(memory, x, y));
	}
	else
	{
		memory->api.pixel(memory, x, y, color);
	}
}
APIFUNC_END

APIFUNC(line)
{
	s32 x0 = POPARG(asX, 0);
	s32 y0 = POPARG(asY, 0);
	s32 x1 = POPARG(asX, 0);
	s32 y1 = POPARG(asY, 0);
	s32 color = POPARG(asColor, 0);
	memory->api.line(memory, x0, y0, x1, y1, color);
}
APIFUNC_END

APIFUNC(rect)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 w = POPARG(asInt, 0);
	s32 h = POPARG(asInt, 0);
	s32 color = POPARG(asColor, 0);
	memory->api.rect(memory, x, y, w, h, color);
}
APIFUNC_END

APIFUNC(rectb)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 w = POPARG(asInt, 0);
	s32 h = POPARG(asInt, 0);
	s32 color = POPARG(asColor, 0);
	memory->api.rect_border(memory, x, y, w, h, color);
}
APIFUNC_END

APIFUNC(spr)
{
	s32 index = POPARG(asInt, 0);
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	cl_object colorkey = POPARG(asObject, ECL_NIL);
	s32 scale = POPARG(asInt, 1);
	tic_flip flip = POPARG(asInt, tic_no_flip);
	tic_rotate rotate = POPARG(asInt, tic_no_rotate);
	s32 w = POPARG(asInt, 1);
	s32 h = POPARG(asInt, 1);
	static u8 colors[TIC_PALETTE_SIZE];
	s32 count = 0;

	if (ECL_FIXNUMP(colorkey))
	{
		colors[0] = asInt(colorkey);
		count = 1;
	}
	else if (ECL_LISTP(colorkey))
	{
		for (s32 i = 0; i < TIC_PALETTE_SIZE; i++)
		{
			if (Null(colorkey))
			{
				break;
			}
			else
			{
				colors[i] = asColor(CAR(colorkey));
				colorkey = CDR(colorkey);
			}
		}
	}

	memory->api.sprite_ex(memory, &memory->ram.gfx, index, x, y, w, h, colors, count, scale, flip, rotate);
}
APIFUNC_END

APIFUNC(btn)
{
	if (machine->memory.input == tic_gamepad_input)
	{
		s32 index = POPARG(asInt, -1);
		if (index == -1)
		{
			RETINT(machine->memory.ram.vram.input.gamepad.data);
		}
		else
		{
			index &= 0x0F;
			RETBOOL(machine->memory.ram.vram.input.gamepad.data & (1 << index));
		}
	}
}
APIFUNC_END

APIFUNC(btnp)
{
	if (machine->memory.input == tic_gamepad_input)
	{
		s32 index = POPARG(asInt, -1);
		s32 hold = POPARG(asInt, -1);
		s32 period = POPARG(asInt, -1);
		if (index != -1)
		{
			index &= 0xF;
		}
		if (hold == -1 && period == -1)
		{
			RETINT(memory->api.btnp(memory, index, hold, period));
		}
		else
		{
			RETBOOL(memory->api.btnp(memory, index, hold, period));
		}
	}
}
APIFUNC_END

APIFUNC(sfx)
{
	s32 index = POPARG(asInt, -1);
	if (index < SFX_COUNT)
	{
		s32 note = -1;
		s32 octave = -1;
		s32 speed = SFX_DEF_SPEED;
		if (index >= 0)
		{
			tic_sound_effect* effect = memory->ram.sound.sfx.data + index;
			note = effect->note;
			octave = effect->octave;
			speed = effect->speed;
		}
		cl_object notespec = POPARG(asObject, ECL_NIL);
		if (ECL_FIXNUMP(notespec))
		{
			s32 id = asInt(notespec);
			note = id % NOTES;
			octave = id / NOTES;
		}
		else if (ECL_STRINGP(notespec))
		{
			const char *noteStr = asText(notespec);
			tic_tool_parse_note(noteStr, &note, &octave);
		}
		s32 duration = POPARG(asInt, -1);
		s32 channel = POPARG(asInt, 0);
		s32 volume = POPARG(asInt, MAX_VOLUME) & 0xF;
		speed = POPARG(asInt, speed);
		if (channel >= 0 && channel < TIC_SOUND_CHANNELS)
		{
			memory->api.sfx_stop(memory, channel);
			memory->api.sfx_ex(memory, index, note, octave, duration, channel, volume, speed);
		}
	}
}
APIFUNC_END

static void remapCallback(void *data, s32 x, s32 y, RemapResult* result)
{
	cl_object fn = (cl_object)data;
	cl_object obj = cl_funcall(4, fn,
				   ecl_make_int32_t(result->index),
				   ecl_make_int32_t(x),
				   ecl_make_int32_t(y));
	if (ECL_CONSP(obj))
	{
		result->index = asInt(CAR(obj));
		result->flip = asInt(CADR(obj));
		result->rotate = asInt(CADDR(obj));
	}
}

APIFUNC(map)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 w = POPARG(asInt, TIC_MAP_SCREEN_WIDTH);
	s32 h = POPARG(asInt, TIC_MAP_SCREEN_HEIGHT);
	s32 sx = POPARG(asInt, 0);
	s32 sy = POPARG(asInt, 0);
	u8 chromakey = POPARG(asInt, -1);
	s32 scale = POPARG(asInt, 1);
	cl_object remap = POPARG(asObject, ECL_NIL);

	if (asBool(cl_functionp(remap)))
	{
		memory->api.remap(memory, &memory->ram.gfx, x, y, w, h, sx, sy, chromakey, scale, remapCallback, remap);
	}
	else
	{
		memory->api.map(memory, &memory->ram.gfx, x, y, w, h, sx, sy, chromakey, scale);
	}
}
APIFUNC_END

APIFUNC(mget)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	RETINT((u8)memory->api.map_get(memory, &memory->ram.gfx, x, y));
}
APIFUNC_END

APIFUNC(mset)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	u8 val = POPARG(asInt, 0);
	memory->api.map_set(memory, &memory->ram.gfx, x, y, val);
}
APIFUNC_END

APIFUNC(peek)
{
	s32 address = POPARG(asInt, -1);
	if (address >= 0 && address < sizeof(tic_ram))
	{
		u8 val = *((u8*)&machine->memory.ram + address);
		RETINT(val);
	}
}
APIFUNC_END

APIFUNC(poke)
{
	s32 address = POPARG(asInt, -1);
	if (address >= 0 && address < sizeof(tic_ram))
	{
		u8 val = POPARG(asInt, 0) & 0xFF;
		*((u8*)&machine->memory.ram + address) = val;
	}
}
APIFUNC_END

APIFUNC(peek4)
{
	s32 address = POPARG(asInt, -1);
	if (address >= 0 && address < sizeof(tic_ram)*2)
	{
		u8 val = tic_tool_peek4((u8*)&machine->memory.ram, address);
		RETINT(val);
	}
}
APIFUNC_END

APIFUNC(poke4)
{
	s32 address = POPARG(asInt, -1);
	if (address >= 0 && address < sizeof(tic_ram)*2)
	{
		u8 val = POPARG(asInt, 0) & 0x0F;
		tic_tool_poke4((u8*)&machine->memory.ram, address, val);
	}
}
APIFUNC_END

APIFUNC(memcpy)
{
	s32 dest = POPARG(asInt, 0);
	s32 src = POPARG(asInt, 0);
	s32 size = POPARG(asInt, 0);
	s32 bound = sizeof(tic_ram) - size;
	if (src >= 0 && size <= sizeof(tic_ram) && dest >= 0 && src >= 0 && dest <= bound && src <= bound)
	{
            u8* base = (u8*)&machine->memory;
            memcpy(base + dest, base + src, size);
	}
}
APIFUNC_END

APIFUNC(memset)
{
	s32 dest = POPARG(asInt, 0);
	u8 val = POPARG(asInt, 0);
	s32 size = POPARG(asInt, 0);
	s32 bound = sizeof(tic_ram) - size;
	if (size >= 0 && size <= sizeof(tic_ram) && dest >= 0 && dest <= bound)
	{
            u8* base = (u8*)&machine->memory;
            memset(base + dest, val, size);
	}
}
APIFUNC_END

APIFUNC(trace)
{
	const char* text = POPARG(asText, "");
	u8 color = POPARG(asColor, tic_color_white);
	machine->data->trace(machine->data->data, text ? text : "nil", color);
}
APIFUNC_END

APIFUNC(pmem)
{
	u32 index = POPARG(asInt, 0);
	index %= TIC_PERSISTENT_SIZE;
	s32 val = memory->ram.persistent.data[index];
	s32 newval = POPARG(asInt, -1);
	if (newval > -1)
	{
		memory->ram.persistent.data[index] = newval;
	}
	RETINT(val);
}
APIFUNC_END

APIFUNC(time)
{
	RETINT(memory->api.time(memory));
}
APIFUNC_END

APIFUNC(exit)
{
	machine->data->exit(machine->data->data);
}
APIFUNC_END

APIFUNC(font)
{
	const char* text = POPARG(asText, "");
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	u8 chromakey = POPARG(asInt, 0);
	s32 w = POPARG(asInt, TIC_SPRITESIZE);
	s32 h = POPARG(asInt, TIC_SPRITESIZE);
	bool fixed = POPARG(asBool, false);
	s32 scale = POPARG(asInt, 1);
	s32 size = 0;
	if (scale > 0)
	{
		size = drawText(memory, text ? text : "nil", x, y, w, h, chromakey, scale,
				fixed ? drawSpriteFont : drawFixedSpriteFont);
	}
	RETINT(size);
}
APIFUNC_END

APIFUNC(mouse)
{
	if (machine->memory.input == tic_mouse_input)
	{
		u16 data = machine->memory.ram.vram.input.gamepad.data;
		cl_object x = ecl_make_int32_t((data & 0x7FFF) % TIC80_WIDTH);
		cl_object y = ecl_make_int32_t((data & 0x7FFF) / TIC80_WIDTH);
		cl_object pressed = (data >> 15) ? ECL_T : ECL_NIL;
		result = CONS(x, CONS(y, CONS(pressed, ECL_NIL)));
	}
}
APIFUNC_END

APIFUNC(circ)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 radius = POPARG(asInt, -1);
	s32 color = POPARG(asColor, 0);
	if (radius >= 0) {
		memory->api.circle(memory, x, y, radius, color);
	}
}
APIFUNC_END

APIFUNC(circb)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 radius = POPARG(asInt, -1);
	s32 color = POPARG(asColor, 0);
	if (radius >= 0)
	{
		memory->api.circle_border(memory, x, y, radius, color);
	}
}
APIFUNC_END

APIFUNC(tri)
{
	s32 pt[6];
	for (s32 i = 0; i < COUNT_OF(pt); i++)
		pt[i] = POPARG(asInt, 0);
	s32 color = POPARG(asColor, 0);
	memory->api.tri(memory, pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], color);
}
APIFUNC_END

APIFUNC(textri)
{
	float pt[12];
	for (s32 i = 0; i < COUNT_OF(pt); i++)
		pt[i] = POPARG(asFloat, 0.0);
	bool use_map = POPARG(asBool, false);
	u8 chroma = POPARG(asInt, 0xFF);
	memory->api.textri(memory,
			   pt[0], pt[1],
			   pt[2], pt[3],
			   pt[4], pt[5],
			   pt[6], pt[7],
			   pt[8], pt[9],
			   pt[10], pt[11],
			   use_map, chroma);
}
APIFUNC_END

APIFUNC(clip)
{
	s32 x = POPARG(asX, 0);
	s32 y = POPARG(asY, 0);
	s32 w = POPARG(asInt, TIC80_WIDTH);
	s32 h = POPARG(asInt, TIC80_HEIGHT);
	memory->api.clip(memory, x, y, w, h);
}
APIFUNC_END

APIFUNC(music)
{
	memory->api.music(memory, -1, 0, 0, false);

	s32 track = POPARG(asInt, -1);
	s32 frame = POPARG(asInt, -1);
	s32 row = POPARG(asInt, -1);
	bool loop = POPARG(asBool, true);

	memory->api.music(memory, track, frame, row, loop);
}
APIFUNC_END

APIFUNC(sync)
{
	memory->api.sync(memory, true);
}
APIFUNC_END

APIFUNC(debugger)
{
    const char* text = POPARG(asText, "CL error :(");
    machine->data->error(machine->data->data, text);
    cl_env_ptr the_env = ecl_process_env();
    if (the_env->frs_org <= the_env->frs_top)
      ecl_unwind(the_env, the_env->frs_org);
}
APIFUNC_END

static void define(const char* name, cl_objectfn fn)
{
	ecl_shadow(ecl_read_from_cstring(name), ecl_current_package());
	ecl_def_c_function_va(ecl_read_from_cstring(name), fn);
}

static const char* const ApiKeywords[] = API_KEYWORDS;
static const cl_objectfn ApiFunc[] =
{
	NULL, NULL, clapi_print, clapi_cls, clapi_pix, clapi_line, clapi_rect,
	clapi_rectb, clapi_spr, clapi_btn, clapi_btnp, clapi_sfx, clapi_map, clapi_mget,
	clapi_mset, clapi_peek, clapi_poke, clapi_peek4, clapi_poke4, clapi_memcpy,
	clapi_memset, clapi_trace, clapi_pmem, clapi_time, clapi_exit, clapi_font, clapi_mouse,
	clapi_circ, clapi_circb, clapi_tri, clapi_textri, clapi_clip, clapi_music, clapi_sync
};

static bool initAPI(tic_machine* machine)
{
	ecl_defvar(ecl_read_from_cstring("**machine**"), ecl_make_pointer(machine));

        define("%debugger", clapi_debugger);
        cl_set(ecl_read_from_cstring("*debugger-hook*"), ecl_read_from_cstring("%debugger"));

	for (s32 i = 0; i < COUNT_OF(ApiFunc); i++)
		if (ApiFunc[i])
			define(ApiKeywords[i], ApiFunc[i]);

	return true;
}

static bool loadCode(tic_machine* machine, const char* code)
{
	int wrappedlen;
	char* wrapped;

	/* Ughhhh.. :( */
	wrappedlen = strlen(code) + 64;
	wrapped = malloc(wrappedlen);
	if (!wrapped)
	{
		return false;
	}
	snprintf(wrapped, wrappedlen, "(progn %s)", code);

	cl_env_ptr env = ecl_process_env();
	ECL_CATCH_ALL_BEGIN(env)
	{
		cl_eval(ecl_read_from_cstring(wrapped));
	}
	ECL_CATCH_ALL_IF_CAUGHT
	{
		machine->data->error(machine->data->data, "problem evaluating CL code :(");
	}
	ECL_CATCH_ALL_END;

	free(wrapped);

	return true;
}

void closeCommonLisp(tic_machine* machine)
{
	if (machine->cl)
	{
		cl_shutdown();
		machine->cl = 0;
	}
}

bool initCommonLisp(tic_machine* machine, const char* code)
{
	int argc = 1;
	char* argv[] = {"cl", NULL};

	closeCommonLisp(machine);

	cl_boot(argc, argv);

	machine->cl = 1;

	if (!initAPI(machine))
	{
		return false;
	}

	if (!loadCode(machine, code))
	{
		return false;
	}

	return true;
}

void callCommonLispTick(tic_machine* machine)
{
	const char* TicFunc = ApiKeywords[0];

	if (machine->cl)
	{
		cl_object sym = ecl_read_from_cstring(TicFunc);
		if (!Null(cl_fboundp(sym)))
		{
			cl_env_ptr env = ecl_process_env();
			ECL_CATCH_ALL_BEGIN(env)
			{
				cl_funcall(1, sym);
			}
			ECL_CATCH_ALL_IF_CAUGHT
			{
				machine->data->error(machine->data->data, "function TIC exited with an error :(");
			}
			ECL_CATCH_ALL_END;
		}
		else
		{
			machine->data->error(machine->data->data, "function TIC isn't found :(");
		}
	}
}

void callCommonLispScanline(tic_mem* memory, s32 row)
{
	tic_machine* machine = (tic_machine*)memory;

	if (machine->cl)
	{
		const char* ScanlineFunc = ApiKeywords[1];

		cl_object sym = ecl_read_from_cstring(ScanlineFunc);
		if (!Null(cl_fboundp(sym)))
		{
			cl_env_ptr env = ecl_process_env();
			ECL_CATCH_ALL_BEGIN(env)
			{
				cl_funcall(1, sym);
			}
			ECL_CATCH_ALL_IF_CAUGHT
			{
				machine->data->error(machine->data->data, "function SCANLINE exited with an error :(");
			}
			ECL_CATCH_ALL_END;
		}
	}
}
