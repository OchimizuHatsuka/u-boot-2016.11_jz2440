#include <common.h>
#include <asm/arch-s3c24x0/s3c2440.h>
#include <asm/arch-s3c24x0/s3c24x0.h>

#define BUSY            1

#define NAND_SECTOR_SIZE    512
#define NAND_BLOCK_MASK     (NAND_SECTOR_SIZE - 1)

#define NAND_SECTOR_SIZE_LP    2048
#define NAND_BLOCK_MASK_LP     (NAND_SECTOR_SIZE_LP - 1)

/* S3C2440��NAND Flash�������� */


/* �ȴ�NAND Flash���� */
static void s3c2440_wait_idle(void)
{
    int i;
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();
    volatile unsigned char *p = (volatile unsigned char *)&nand_chip->nfstat;

    while(!(*p & BUSY))
        for(i=0; i<10; i++);
}

/* ����Ƭѡ�ź� */
static void s3c2440_nand_select_chip(void)
{
    int i;
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();

    nand_chip->nfcont &= ~(1<<1);
    for(i=0; i<10; i++);    
}

/* ȡ��Ƭѡ�ź� */
static void s3c2440_nand_deselect_chip(void)
{
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();

    nand_chip->nfcont |= (1<<1);
}

/* �������� */
static void s3c2440_write_cmd(int cmd)
{
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();

    volatile unsigned char *p = (volatile unsigned char *)&nand_chip->nfcmd;
    *p = cmd;
}

/* ������ַ */
static void s3c2440_write_addr(unsigned int addr)
{
    int i;
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();
    volatile unsigned char *p = (volatile unsigned char *)&nand_chip->nfaddr;
    
    *p = addr & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 9) & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 17) & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 25) & 0xff;
    for(i=0; i<10; i++);
}


/* ������ַ */
static void s3c2440_write_addr_lp(unsigned int addr)
{
    int i;
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();
    volatile unsigned char *p = (volatile unsigned char *)&nand_chip->nfaddr;
	int col, page;

	col = addr & NAND_BLOCK_MASK_LP;
	page = addr / NAND_SECTOR_SIZE_LP;
	
    *p = col & 0xff;			/* Column Address A0~A7 */
    for(i=0; i<10; i++);		
    *p = (col >> 8) & 0x0f;		/* Column Address A8~A11 */
    for(i=0; i<10; i++);
    *p = page & 0xff;			/* Row Address A12~A19 */
    for(i=0; i<10; i++);
    *p = (page >> 8) & 0xff;	/* Row Address A20~A27 */
    for(i=0; i<10; i++);
    *p = (page >> 16) & 0x03;	/* Row Address A28~A29 */
    for(i=0; i<10; i++);
}

/* ��ȡ���� */
static unsigned char s3c2440_read_data(void)
{
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();
    volatile unsigned char *p = (volatile unsigned char *)&nand_chip->nfdata;
    return *p;
}

/* ��λ */
static void s3c2440_nand_reset(void)
{
    s3c2440_nand_select_chip();
    s3c2440_write_cmd(0xff);  // ��λ����
    s3c2440_wait_idle();
    s3c2440_nand_deselect_chip();
}
/* �ڵ�һ��ʹ��NAND Flashǰ����λһ��NAND Flash */
static void nand_reset(void)
{
    s3c2440_nand_reset();
}

static void wait_idle(void)
{
    s3c2440_wait_idle();
}

static void nand_select_chip(void)
{
    int i;
	
    s3c2440_nand_select_chip();
	
    for(i=0; i<10; i++);
}

static void nand_deselect_chip(void)
{
	s3c2440_nand_deselect_chip();
}

static void write_cmd(int cmd)
{
    s3c2440_write_cmd(cmd);
}
static void write_addr(unsigned int addr)
{
    s3c2440_write_addr(addr);
}

static void write_addr_lp(unsigned int addr)
{
    s3c2440_write_addr_lp(addr);
}

static unsigned char read_data(void)
{
    return s3c2440_read_data();
}

/* ��ʼ��NAND Flash */
void nand_init_ll(void)
{
	struct s3c24x0_nand * nand_chip = s3c24x0_get_base_nand();

#define TACLS   0
#define TWRPH0  4
#define TWRPH1  2

		/* ����ʱ�� */
    nand_chip->nfconf = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
        /* ʹ��NAND Flash������, ��ʼ��ECC, ��ֹƬѡ */
    nand_chip->nfcont = (1<<4)|(1<<1)|(1<<0);

	/* ��λNAND Flash */
	nand_reset();
}


/* ������ */
void nand_read_ll(unsigned char *buf, unsigned long start_addr, int size)
{
    int i, j;
    
    if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK)) {
        return ;    /* ��ַ�򳤶Ȳ����� */
    }

    /* ѡ��оƬ */
    nand_select_chip();

    for(i=start_addr; i < (start_addr + size);) {
      /* ����READ0���� */
      write_cmd(0);

      /* Write Address */
      write_addr(i);
      wait_idle();

      for(j=0; j < NAND_SECTOR_SIZE; j++, i++) {
          *buf = read_data();
          buf++;
      }
    }

    /* ȡ��Ƭѡ�ź� */
    nand_deselect_chip();
    
    return ;
}


/* ������ 
  * Large Page
  */
void nand_read_ll_lp(unsigned char *buf, unsigned long start_addr, int size)
{
    int i, j;
    
    if ((start_addr & NAND_BLOCK_MASK_LP) || (size & NAND_BLOCK_MASK_LP)) {
        return ;    /* ��ַ�򳤶Ȳ����� */
    }

    /* ѡ��оƬ */
    nand_select_chip();

    for(i=start_addr; i < (start_addr + size);) {
      /* ����READ0���� */
      write_cmd(0);

      /* Write Address */
      write_addr_lp(i);
	  write_cmd(0x30);
      wait_idle();

      for(j=0; j < NAND_SECTOR_SIZE_LP; j++, i++) {
          *buf = read_data();
          buf++;
      }
    }

    /* ȡ��Ƭѡ�ź� */
    nand_deselect_chip();
    
    return ;
}

void nand_init(void)
{
    /* ��ʼ��NAND Flash */
    nand_init_ll();
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
    /* �� NAND Flash���� */
    nand_read_ll_lp(dst, offs, (size + NAND_BLOCK_MASK_LP)&~(NAND_BLOCK_MASK_LP));
	return 0;
}

/* Unselect after operation */
void nand_deselect(void)
{
}

