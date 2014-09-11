
#define PROTOCOL_ID     0x40

/* CMD_R_: read from tag command
 * CMD_W_: write to tag command
 */
typedef enum {
    CMD_R_COLLECTION_UDB    = 0x1f,
    CMD_W_SLEEP             = 0x15,   /* 6.3.2 */
    CMD_W_SLEEP_ALL_BUT     = 0x16,     /* 6.3.3 */
    CMD_R_FIRMWARE_VERSION  = 0x0c,
    CMD_R_MODEL_NUM         = 0x0e,
    CMD_W_MEMORY            = 0xe0,
    CMD_R_MEMORY            = 0x60,
    CMD_W_DELETE            = 0x8e,
    CMD_R_UDB               = 0x70,
    CMD_W_TABLE             = 0x26,
} iso18000_cmd_e;

typedef enum {
    ERROR_INVALID_COMMAND_CODE = 0x01,
} iso8000_error_e;

typedef union {
    struct {
        ot_u16 service  : 1;    // 0
        ot_u16 res_a    : 2;    // 1,2
        ot_u16 tag_type : 3;    // 3,4,5
        ot_u16 res_b    : 2;    // 6,7
        ot_u16 nack     : 1;    // 8
        ot_u16 res_c    : 2;    // 9,10
        ot_u16 alarm    : 1;    // 11
        ot_u16 mode     : 4;    // 12,13,4,15       table 26, response to broadcast or ptp
    } bits;
    ot_u16 word;
} tag_status_t;

typedef enum {  // table 40
    UDB_TYPECODE_TRANSIT_DATA   = 0x00, // routing code element, user ID element
    UDB_TYPECODE_CAPABILITY_DATA,   //  capability elements
    UDB_TYPECODE_QUERY_RESULTS,     // table query results element
    UDB_TYPECODE_HARDWARE_FAULT_DATA    // hardware fault status element
} udb_typecode_e;

typedef enum { // table 30
    UDB_ELEMENT_TYPE__ROUTING_CODE = 0x10,  // ISO 17363 
    UDB_ELEMENT_TYPE__USER_ID,
    UDB_ELEMENT_TYPE__APPLICATION = 0xff,   // extension
} udb_element_type_e;

