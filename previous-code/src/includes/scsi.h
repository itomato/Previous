/* SCSI Bus and Disk emulation */

/* SCSI phase */
#define PHASE_DO      0x00 /* data out */
#define PHASE_DI      0x01 /* data in */
#define PHASE_CD      0x02 /* command */
#define PHASE_ST      0x03 /* status */
#define PHASE_MO      0x06 /* message out */
#define PHASE_MI      0x07 /* message in */

typedef struct {
    uint8_t target;
    uint8_t phase;
} SCSIBusStatus;

extern SCSIBusStatus SCSIbus;


/* Command Descriptor Block */
#define SCSI_CDB_MAX_SIZE 12


/* This buffer temporarily stores data to be written to memory or disk */

typedef struct {
    uint8_t data[512]; /* FIXME: BLOCKSIZE */
    int limit;
    int size;
    bool disk;
    int64_t time;
} SCSIBuffer;

extern SCSIBuffer scsi_buffer;


void SCSI_Init(void);
void SCSI_Uninit(void);
void SCSI_Reset(void);
void SCSI_Insert(uint8_t target);
void SCSI_Eject(uint8_t target);

uint8_t SCSIdisk_Send_Status(void);
uint8_t SCSIdisk_Send_Message(void);
uint8_t SCSIdisk_Send_Data(void);
void SCSIdisk_Receive_Data(uint8_t val);
bool SCSIdisk_Select(uint8_t target);
void SCSIdisk_Receive_Command(uint8_t *commandbuf, uint8_t identify);

int64_t SCSIdisk_Time(void);
