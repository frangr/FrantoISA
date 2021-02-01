//FrantoISA cpu emulator
#include "stdint-gcc.h"
#include "stdio.h"


#define RESET_VEC 0b1100
#define BO inst >> 24
#define SB inst >> 16 & 0xff
#define BT inst >> 8 & 0xff
#define BF inst & 0xff

#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 5
#define LB  19
#define LH  6
#define LW  7
#define SBY  8
#define SH  9
#define SW 10
#define JE 11
#define JNE 12
#define JG 13
#define JNG 14
#define JMP 15
#define JMPS 16
#define LIRH 17
#define LIRL 18
#define LDR 4
#define MVR 20
#define AND 21
#define OR 22
#define XOR 23
#define STBI 24
#define SHL 25
#define SHR 26
#define JMPC 28
#define DEBUG_STOP 29

char debug_stop = 0;
uint32_t* register_monitor;
uint8_t disp[80];
uint8_t memory[3000];

uint32_t fetch(uint8_t* memory, char addr)
{
    return memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
}

uint8_t fetch_byte(char* memory, char addr)
{
    return memory[addr];
}

void display_io(uint8_t data, short addr)
{
    if(addr > 79)
        return;

    disp[addr] = data;
}

void draw_display()
{
    for(char i = 4; i < 4; i++)
        for(char i = 20; i < 20; i++)
            printf("%u",disp[i]);
}

void send_chipset(short addr, uint32_t* data, char rw, char data_size)
{
    if(addr > 2549 && addr < 2581)
    {
        //80 byte video memory
        printf("IO!\n");
        return;
    }
    //LOAD
    if(!rw)
    {
        if(!data_size)
            *data = fetch(memory, addr);
        else if(data_size == 1)
            *data = (fetch(memory, addr) << 8) | fetch(memory, addr+1);
        else if(data_size == 2)
            *data = fetch_byte(memory, addr);
    }
    else if(rw == 1) //WRITE
    {
        if(!data_size)
        {
            memory[addr]   = *data >> 24;
            memory[addr+1] = *data >> 16;
            memory[addr+2] = *data >> 8;
            memory[addr+3] = *data;
        }
        else if(data_size == 1)
        {
            memory[addr]   = *data >> 8;
            memory[addr+1] = *data;
        }
        else if(data_size == 2)
            memory[addr] = *data;
    }
}

char control(uint8_t cnt, uint8_t reg_check)
{
    switch(cnt)
    {

    }
}

void cpu(char* memory)
{
    //program counter
    static uint32_t PC = 0;
    //if 0, cpu just started running
    static char reset = 0;
    //cpu registers
    static uint32_t reg[16];

    //0 = 32, 1 = 16, 2 = 8
    char mem_op_l = 0;

    if(!reset)
    {
        for(char i = 0; i < 16; i++)
            reg[i] = 0;
        register_monitor = reg;
        send_chipset(0, &PC, 0, 0);
        reset = 1;
    }

    uint32_t inst;
    send_chipset(PC, &inst, 0, 0);

    uint32_t buffer;
    int16_t offset;
    switch(inst >> 24)
    {
        case ADD:
            reg[BF] = reg[SB] + reg[BT];
            PC += 4;
            break;
        case SUB:
            reg[BF] = reg[SB] - reg[BT];
            PC += 4;
            break;
        case MUL:
            reg[BF] = reg[SB] * reg[BT];
            PC += 4;
            break;
        case DIV:
            reg[BF] = reg[SB] / reg[BT];
            PC += 4;
            break;
        case LB:
            printf("lb\n");
            if(BF)
                if(!control(BF, BT))
                    break;

            //reg[BT] = send_chipset(memory, reg[SB]);
            send_chipset(reg[SB], &reg[BT], 0, 2);
            PC += 4;
            break;
        case LH:
            if(BF)
                if(!control(BF, BT))
                    break;

            send_chipset(reg[SB], &reg[BT], 0, 1);
            PC += 4;
            break;
        case LW:
            if(BF)
                if(!control(BF, BT))
                    break;

            send_chipset(reg[SB], &reg[BT], 0, 0);
            PC += 4;
            break;
        case SBY: //address SB, data BT
            if(BF)
                if(!control(BF, BT))
                    break;
                printf("sby\n");
             //send_chipset_write(memory, reg[SB], reg[BT]);
             send_chipset(reg[SB], &reg[BT], 1, 2);
             PC += 4;
            break;
        case SH:
            if(BF)
                if(!control(BF, BT))
                    break;

             send_chipset(reg[SB], &reg[BT], 1, 1);
             PC += 4;
            break;
        case SW:
            if(BF)
                if(!control(BF, BT))
                    break;

             send_chipset(reg[SB], &reg[BT], 1, 0);
             PC += 4;
            break;
        case JE:
            if(reg[SB] == reg[BT])
                PC = reg[BF];
            PC += 4;
            break;
        case JNE:
            if(reg[SB] != reg[BT])
                PC = reg[BF];
            PC += 4;
            break;
        case JG:
            if(reg[SB] > reg[BT])
                PC = reg[BF];
            PC += 4;
            break;
        case JNG:
            if(reg[SB] < reg[BT])
                PC = reg[BF];
            PC += 4;
            break;
        case JMP:
            offset = inst & 0xffffff;
            PC += offset;
            break;
        case JMPS:
            printf("jmps\n");
            reg[SB] = PC;
            PC = reg[BT];
            break;
        case LIRH:
            printf("lirh \n");
            reg[SB] |= inst << 16;
            PC += 4;
            break;
        case LIRL:
            printf("lirl\n");
            reg[SB] |= inst & 0xffff;
            PC += 4;
            break;
        case LDR:
            printf("ldr\n");
            send_chipset(PC + (inst >> 24), &buffer, 0, 0);

            reg[0] = inst << 8;
            reg[0] |= buffer >> 24; //PC+5);
            //reg[0] = inst << 8 | fetch_byte(memory, PC + inst >> 24); //PC+5);
            PC += 5;
            break;
        case MVR:
            printf("mvr\n");
            if(BF)
                reg[BF] = reg[SB];
            reg[SB] = reg[BT];

            PC += 4;
            break;
        case AND:
            reg[BF] = reg[SB] & reg[BT];
            PC += 4;
            break;
        case OR:
            reg[BF] = reg[SB] | reg[BT];
            PC += 4;
            break;
        case XOR:
            reg[BF] = reg[SB] ^ reg[BT];
            PC += 4;
            break;
        case STBI:
            reg[SB] |= (reg[BF] & 0b1) << reg[BT];
            PC += 4;
            break;
        case SHL:
            if(reg[BF])
                reg[BF] = reg[SB];

            reg[SB] = reg[SB] << reg[BT];
            PC += 4;
            break;
        case SHR:
            if(reg[BF])
                reg[BF] = reg[SB];

            reg[SB] = reg[SB] >> reg[BT];
            PC += 4;
            break;
        case JMPC:
            if(BT)
                if(!control(BF, BT))
                    break;

            PC = reg[BF];
            break;
        case DEBUG_STOP:
            debug_stop = 1;
            return;
        default:
            printf("OPCODE INVALID: %u (quitting)\n", inst >> 24);
            debug_stop = 1;
            break;
    }
}

void system_FrantoISA(char* memory)
{
    cpu(memory);
    draw_display();
}

void memory_load4B(uint32_t data, short* addr)
{
    memory[*addr] = data >> 24;
    memory[*addr+1] = (data >> 16) & 0xff;
    memory[*addr+2] = (data >> 8) & 0xff;
    memory[*addr+3] = data;
    *addr += 4;
}

void memory_load5B(uint32_t data, uint8_t byte, short* addr)
{
    memory[*addr] = data >> 24;
    memory[*addr+1] = (data >> 16) & 0xff;
    memory[*addr+2] = (data >> 8) & 0xff;
    memory[*addr+3] = data;
    memory[*addr+4] = byte;
    *addr += 5;
}

void main()
{
    //storing reset vector (address 12)
    memory[0] = RESET_VEC >> 24;
    memory[1] = RESET_VEC >> 16;
    memory[2] = RESET_VEC >> 8;
    memory[3] = RESET_VEC;

    short start_addr = RESET_VEC;

    memory_load4B(0x12040A00, &start_addr);
    memory_load4B(0x12020050, &start_addr);
    memory_load4B(0x11040001, &start_addr);
    memory_load5B(0x04ABE29C, 0xAD, &start_addr);
    memory_load4B(0x08040500, &start_addr);
    memory_load4B(0x08040500, &start_addr);
    memory_load4B(0x13020000, &start_addr);
    memory_load4B(DEBUG_STOP << 24, &start_addr);

    memory[80] = 53;

    while(!debug_stop)
        system_FrantoISA(memory);

    for(char i = 0; i < 16; i++)
        printf("val[%u]: %u\n", i, register_monitor[i]);

    printf("ADDR current: %u\n", start_addr);

    printf("execution ended");
}
