#include <stdint.h>
#include <stdbool.h>

struct efi_table_header {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;
};

struct efi_system_table {
	struct efi_table_header header;
	uint16_t *unused1;
	uint32_t unused2;
	void *unused3;
	void *unused4;
	void *unused5;
	struct efi_simple_text_output_protocol *out;
	void *unused6;
	void *unused7;
	struct efi_boot_services *boot_services;
	void *unused9;
	uint64_t unused10;
	void *unused11;
};

typedef uint64_t efi_status_t;
typedef uint64_t efi_uint_t;

struct efi_simple_text_output_protocol {
	efi_status_t (*unused1)(
			struct efi_simple_text_output_protocol *,
			bool);

	efi_status_t (*output_string)(
		struct efi_simple_text_output_protocol *self,
		uint16_t *string);

	efi_status_t (*unused2)(
		struct efi_simple_text_output_protocol *,
		uint16_t *);
	efi_status_t (*unused3)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t, efi_uint_t *, efi_uint_t *);
	efi_status_t (*unused4)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t);
	efi_status_t (*unused5)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t);

	efi_status_t (*clear_screen)(
		struct efi_simple_text_output_protocol *self);

	efi_status_t (*unused6)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t, efi_uint_t);
	efi_status_t (*unused7)(
		struct efi_simple_text_output_protocol *,
		bool);

	void *unused8;
};

struct efi_boot_services {
    struct efi_table_header header;

    efi_status_t (*raise_tpl)(efi_uint_t);
    void (*restore_tpl)(efi_uint_t);

    efi_status_t (*allocate_pages)(
        efi_uint_t,
        efi_uint_t,
        efi_uint_t,
        uint64_t *
    );

    efi_status_t (*free_pages)(
        uint64_t,
        efi_uint_t
    );

    efi_status_t (*get_memory_map)(
        efi_uint_t *,
        void *,
        uint64_t *,
        efi_uint_t *,
        uint32_t *
    );

    efi_status_t (*allocate_pool)(
        efi_uint_t,
        efi_uint_t,
        void **
    );

    efi_status_t (*free_pool)(
        void *
    );

    /* More functions go here in the exact UEFI-specified order. */
};

typedef void *efi_handle_t;

efi_status_t efi_main(
	efi_handle_t handle, struct efi_system_table *system_table)
{
	    uint16_t msg[] = {
        'H','e','l','l','o',' ',
        'U','E','F','I','!',
        '\r','\n',0
    };

    system_table->out->output_string(
        system_table->out,
        msg
    );

    struct efi_boot_services *bs =
        system_table->boot_services;

    void *buffer;

    efi_status_t status = bs->allocate_pool(
        2,          // EfiLoaderData
        1024,
        &buffer
    );

    if (status != 0)
        return status;

    /* Use buffer... */

    bs->free_pool(buffer);

    while (true) {
    }
	
	return 0;
}