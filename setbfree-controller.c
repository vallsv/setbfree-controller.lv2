/*
 */

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
	MIDI_CONTROL_CHANGE = 0xB0,
} MidiSpec;

typedef enum {
	PORT_ATOM_IN = 0,
    PORT_ATOM_OUT,
    PORT_CONTROL_DRAWBAR_UPPER = 2,
    PORT_CONTROL_DRAWBAR_UPPER_16 = 2,
    PORT_CONTROL_DRAWBAR_UPPER_513,
    PORT_CONTROL_DRAWBAR_UPPER_8,
    PORT_CONTROL_DRAWBAR_UPPER_4,
    PORT_CONTROL_DRAWBAR_UPPER_223,
    PORT_CONTROL_DRAWBAR_UPPER_2,
    PORT_CONTROL_DRAWBAR_UPPER_135,
    PORT_CONTROL_DRAWBAR_UPPER_113,
    PORT_CONTROL_DRAWBAR_UPPER_1,
	// Note: it have to be the last
	PORT_ENUM_SIZE
} PortEnum;

typedef struct {
	uint8_t channel;
	uint8_t control;
} SetBFreeMidiConfig;

typedef struct {
    const float* port;
	float last_value;
	SetBFreeMidiConfig midi_config;
} Parameter;

typedef struct {
    // URIDs
    LV2_URID urid_midiEvent;

    // state
    // Note: it also alloc slots for MIDI in/out but we dont care
    Parameter parameters[PORT_ENUM_SIZE];

    // Forge
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame frame;

    // atom ports
    const LV2_Atom_Sequence* port_events_in;
    LV2_Atom_Sequence* port_events_out;
} Data;


static LV2_Handle instantiate(const LV2_Descriptor*     descriptor,
                              double                    rate,
                              const char*               path,
                              const LV2_Feature* const* features)
{
    Data* self = (Data*)calloc(1, sizeof(Data));

    for (int port = PORT_CONTROL_DRAWBAR_UPPER_16;
         port <= PORT_CONTROL_DRAWBAR_UPPER_1; port++) {
    	Parameter *parameter = self->parameters + port;
    	parameter->last_value = -1;
    }
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+0].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 70 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+1].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 71 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+2].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 72 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+3].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 73 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+4].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 74 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+5].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 75 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+6].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 76 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+7].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 77 };
    self->parameters[PORT_CONTROL_DRAWBAR_UPPER+8].midi_config = (SetBFreeMidiConfig) { .channel = 0, .control = 78 };

    // Get host features
    LV2_URID_Map* urid_map = NULL;

    for (int i = 0; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_URID__map)) {
        	urid_map = (LV2_URID_Map*)features[i]->data;
            break;
        }
    }

    if (!urid_map) {
        free(self);
        return NULL;
    }

    // Map URIs
    self->urid_midiEvent = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);
    lv2_atom_forge_init(&self->forge, urid_map);

    return self;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data)
{
    Data* self = (Data*)instance;

    switch (port)
    {
    case PORT_ATOM_IN:
            self->port_events_in = (const LV2_Atom_Sequence*)data;
            break;
    case PORT_ATOM_OUT:
            self->port_events_out = (LV2_Atom_Sequence*)data;
            break;
    default:
		self->parameters[port].port = (const float*)data;
    }
}

static void activate(LV2_Handle instance)
{
    // Data* self = (Data*)instance;
}

static void run(LV2_Handle instance, uint32_t sample_count)
{
	Data* self = (Data*)instance;
    LV2_Atom midiatom;
    uint8_t msg_buffer[3 * PORT_ENUM_SIZE];
    uint8_t *msg;

    msg = msg_buffer;
    for (int port = PORT_CONTROL_DRAWBAR_UPPER_16;
         port <= PORT_CONTROL_DRAWBAR_UPPER_1; port++) {
    	Parameter *parameter = self->parameters + port;

    	if (parameter->last_value == *parameter->port) {
    		continue;
    	}
    	parameter->last_value = *parameter->port;

    	// make the event
        msg[0] = MIDI_CONTROL_CHANGE + parameter->midi_config.channel;
        msg[1] = parameter->midi_config.control;
        msg[2] = 127 - (int) ((parameter->last_value * 127) / 8);
    	msg += 3;
    }

    if (msg != msg_buffer) {
        // get MIDI port ready
        const uint32_t capacity = self->port_events_out->atom.size;
        lv2_atom_forge_set_buffer(&self->forge, (uint8_t*)self->port_events_out, capacity);
        lv2_atom_forge_sequence_head(&self->forge, &self->frame, 0);
        midiatom.type = self->urid_midiEvent;
		midiatom.size = msg - msg_buffer;

		// send the event to the atom bus
        lv2_atom_forge_frame_time(&self->forge, 0);
        lv2_atom_forge_raw(&self->forge, &midiatom, sizeof(LV2_Atom));
        lv2_atom_forge_raw(&self->forge, msg_buffer, midiatom.size);
        lv2_atom_forge_pad(&self->forge, midiatom.size + sizeof(LV2_Atom));
    }
}

void _run(LV2_Handle instance, uint32_t sample_count)
{
    Data* self = (Data*)instance;

    // Get the capacity
    const uint32_t out_capacity = self->port_events_out->atom.size;

    // Write an empty Sequence header to the outputs
    lv2_atom_sequence_clear(self->port_events_out);

    // LV2 is so nice...
    self->port_events_out->atom.type = self->port_events_in->atom.type;

    // Read incoming events
    LV2_ATOM_SEQUENCE_FOREACH(self->port_events_in, ev)
    {
        if (ev->body.type != self->urid_midiEvent) {
        	continue;
        }
		lv2_atom_sequence_append_event(self->port_events_out,
									   out_capacity,
									   ev);
    }
}

static void cleanup(LV2_Handle instance)
{
    free(instance);
}

static const LV2_Descriptor descriptor = {
    .URI = "https://github.com/vallsv/setbfree-controller",
    .instantiate = instantiate,
    .connect_port = connect_port,
    .activate = activate,
    .run = run,
    .deactivate = NULL,
    .cleanup = cleanup,
    .extension_data = NULL
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    return (index == 0) ? &descriptor : NULL;
}
