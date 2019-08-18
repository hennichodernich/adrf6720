#ifndef ADRF6720_H
#define ADRF6720_H

#define ADRF6720_SOFT_RESET  0x00
#define ADRF6720_ENABLES     0x01
#define ADRF6720_INT_DIV     0x02
#define ADRF6720_FRAC_DIV    0x03
#define ADRF6720_MOD_DIV     0x04
#define ADRF6720_ENBL_MASK   0x10
#define ADRF6720_CP_CTL      0x20
#define ADRF6720_PFD_CTL     0x21
#define ADRF6720_VCO_CTL     0x22
#define ADRF6720_DGA_CTL     0x23 //ADRF6820 only
#define ADRF6720_BALUN_CTL   0x30
#define ADRF6720_MIXER_CTL   0x31
#define ADRF6720_MOD_LIN_CTL 0x31
#define ADRF6720_MOD_CTL0    0x32
#define ADRF6720_MOD_CTL1    0x33
#define ADRF6720_MOD_CTL2    0x34 //ADRF6820 only
#define ADRF6720_PFD_CP_CTL  0x40
#define ADRF6720_DITH_CTL1   0x42
#define ADRF6720_DITH_CTL2   0x43
#define ADRF6720_CALIBRATION 0x44
#define ADRF6720_VCO_CTL2    0x45
#define ADRF6720_VCO_RB      0x46 //ADRF6820 only
#define ADRF6720_REG47       0x47
#define ADRF6720_REG48       0x48
#define ADRF6720_VCO_CTL3    0x49
#define ADRF6720_SCAN        0x60
#define ADRF6720_VERSION1    0xFE
#define ADRF6720_VERSION2    0xFF

//0x00 SOFT_RESET
#define ADRF6720_FLAG_SOFT_RESET        0x0001
//0x01 ENABLES
#define ADRF6720_FLAG_LO_1XVCO_EN       0x0800
#define ADRF6720_FLAG_MOD_EN            0x0400
#define ADRF6720_FLAG_QUAD_DIV_EN       0x0200
#define ADRF6720_FLAG_LO_DRV2X_EN       0x0100
#define ADRF6720_FLAG_LO_DRV1X_EN       0x0080
#define ADRF6720_FLAG_VCO_MUX_EN        0x0040
#define ADRF6720_FLAG_REF_BUF_EN        0x0020
#define ADRF6720_FLAG_VCO_EN            0x0010
#define ADRF6720_FLAG_DIV_EN            0x0008
#define ADRF6720_FLAG_CP_EN             0x0004
#define ADRF6720_FLAG_VCO_LDO_EN        0x0002
//0x02 INT_DIV
#define ADRF6720_FLAG_DIV_MODE          0x0800
#define ADRF6720_BITS_INT_DIV(n)        (n & 0x03ff)
//0x10 ENBL_MASK
#define ADRF6720_FLAG_LO_1XVCO_MASK     0x0800
#define ADRF6720_FLAG_MOD_MASK          0x0400
#define ADRF6720_FLAG_QUAD_DIV_MASK     0x0200
#define ADRF6720_FLAG_LO_DRV2X_MASK     0x0100
#define ADRF6720_FLAG_LO_DRV1X_MASK     0x0080
#define ADRF6720_FLAG_VCO_MUX_MASK      0x0040
#define ADRF6720_FLAG_REF_BUF_MASK      0x0020
#define ADRF6720_FLAG_VCO_MASK          0x0010
#define ADRF6720_FLAG_DIV_MASK          0x0008
#define ADRF6720_FLAG_CP_MASK           0x0004
#define ADRF6720_FLAG_VCO_LDO_MASK      0x0002
//0x20 CP_CTL
#define ADRF6720_FLAG_CP_SEL            0x4000
#define ADRF6720_BITS_CP_CSCALE(n)      ((n & 0x0f) << 10)
#define ADRF6720_BITS_CP_FSCALE(n)      ((n & 0x0f) << 6)
#define ADRF6720_BITS_CP_BLEED(n)       (n & 0x3f)
//0x21 PFD_CTL
#define ADRF6720_BITS_REF_MUX_SEL(n)    ((n & 0x07) << 4)
#define ADRF6720_FLAG_PFD_POLARITY      0x0008
#define ADRF6720_BITS_REF_SEL(n)        (n & 0x07)
//0x22 VCO_CTL
#define ADRF6720_BITS_VCO_LDO_R4SEL(n)  ((n & 0x0f) << 12)
#define ADRF6720_BITS_VCO_LDO_R2SEL(n)  ((n & 0x0f) << 8)
#define ADRF6720_BITS_LO_DRV_LVL(n)     ((n & 0x03) << 6)
#define ADRF6720_FLAG_DRVDIV2_EN        0x0020
#define ADRF6720_FLAG_DIV8_EN           0x0010
#define ADRF6720_FLAG_DIV4_EN           0x0008
#define ADRF6720_BITS_VCO_SEL(n)        (n & 0x07)
//0x23 DGA_CTL
#define ADRF6720_FLAG_RFSW_MUX          0x0800
#define ADRF6720_FLAG_RFSW_SEL          0x0200
#define ADRF6720_BITS_RFDSA_SEL(n)      ((n & 0x0f) << 5)
//0x30 BALUN_CTL
#define ADRF6720_BITS_BAL_COUT(n)       ((n & 0x0f) << 4)
#define ADRF6720_BITS_BAL_CIN(n)        (n & 0x0f)
//0x31 MIXER_CTL
#define ADRF6720_BITS_MIX_BIAS(n)       ((n & 0x07) << 10)
#define ADRF6720_BITS_DEMOD_RDAC(n)     ((n & 0x0f) << 5)
#define ADRF6720_BITS_DEMOD_CDAC(n)     (n & 0x0f)
//0x31 MOD_LIN_CTL
#define ADRF6720_BITS_MOD_RSEL(n)       ((n & 0x7f) << 6)
#define ADRF6720_BITS_MOD_CSEL(n)       (n & 0x3f)
//0x32 MOD_CTL0
#define ADRF6720_BITS_MOD_BLEED(n)      ((n & 0x07) << 12)
#define ADRF6720_BITS_POL_Q(n)          ((n & 0x03) << 10)
#define ADRF6720_BITS_POL_I(n)          ((n & 0x03) << 8)
#define ADRF6720_BITS_Q_LO(n)           ((n & 0x0f) << 4)
#define ADRF6720_BITS_I_LO(n)           (n & 0x0f)
//0x33 MOD_CTL1
#define ADRF6720_BITS_DCOFF_I(n)        ((n & 0xff) << 8)
#define ADRF6720_BITS_DCOFF_Q(n)        (n & 0xff)
//0x34 MOD_CTL2
#define ADRF6720_BITS_BB_BIAS(n)        ((n & 0x03) << 10)
#define ADRF6720_BITS_BWSEL(n)          ((n & 0x03) << 8)
//0x40 PFD_CP_CTL
#define ADRF6720_BITS_ABLDLY(n)         ((n & 0x03) << 5)
#define ADRF6720_BITS_CP_CTRL(n)        ((n & 0x07) << 2)
#define ADRF6720_BITS_PFD_CLK_EDGE(n)   (n & 0x03)
//0x42 DITH_CTL1
#define ADRF6720_FLAG_DITH_EN           0x0008
#define ADRF6720_BITS_DITH_MAG(n)       ((n & 0x03) << 1)
#define ADRF6720_BITS_DITH_VAL16        0x0001
//0x44 CALIBRATION
#define ADRF6720_FLAG_SDM_DIVD_CLR      0x02
#define ADRF6720_FLAG_BANDCAL		        0x01
//0x45 VCO_CTL2
#define ADRF6720_BITS_VTUNE_CTRL(n)     ((n & 0x03) << 8)
#define ADRF6720_FLAG_VCO_BAND_SRC      0x0080
#define ADRF6720_BITS_BAND(n)           (n & 0x7f)
//0x46 VCO_RB
#define ADRF6720_VCO_BAND(n)            (n & 0x3f)
//0x49 VCO_CTL3
#define ADRF6720_BITS_SET_1(n)          ((n & 0x1f) << 9)
#define ADRF6720_BITS_VTUNE_DAC_SLOPE(n) ((n & 0x1f) << 9)
#define ADRF6720_BITS_SET_0(n)          (n & 0x001ff)
#define ADRF6720_BITS_VTUNE_DAC_OFFSET(n) (n & 0x001ff)
//0x60 UNDOCUMENTED
#define ADRF6720_FLAG_SCAN_EN		0x01


typedef struct{
    uint8_t reg;
    char *name;
}t_regnamepair;

typedef struct{
  int LO_DRV1X_ENLO_DRV1X_EN;
  int LO_DRV2X_EN;
  int DIV_MODE;
  int INT;
  int FRAC;
  int MOD;
  int ENOP;
  int cscale_val;
  int BLEED;
  int FSCALE;
  int REF_MUX_SEL;
  int PFD_Polarity;
  int REF_SEL;
  int VCO_SEL;
  int LO_DRV_LVL;
  int DRVDIV2_EN;
  int lo_divider;
  int DIV8_EN;
  int DIV4_EN;
  int VCO_LDO_R2SEL;
  int VCO_LDO_R4SEL;
  int RFSW_MUX;
  int RFSW_SEL;
  int RFDSA_SEL;
  int BAL_COUT;
  int BAL_CIN;
  int MOD_CSEL;
  int MOD_RSEL;
  int MIX_BIAS;
  int DEMOD_CDAC;
  int DEMOD_RDAC;
  int POLi;
  int POLq;
  int QLO;
  int ILO;
  int DCOFFI;
  int DCOFFQ;
  int BB_BIAS;
  int BWSEL;
  int CLKEDGE;
  int CPCTRL;
  int ABLDLY;
  int DITH_EN;
  int DITH_MAG;
  int DITH_VAL;
  int SDM_DIVD_CLR;
  int BANDCAL;
  int VTUNE_CTRL;
  int VCO_BAND_SRC;
  int BAND;
  int VCO_BAND;
  int VCO_CNTR_DONE;
  int VTUNE_DAC_OFFSET;
  int VTUNE_DAC_SLOPE;
  int SCAN_EN;
  double pll_ref_in;
  double pll_ref_div;
  double pfd_freq;
  double lo_out_freq;
  double vco_freq;
  double tune_freq;
}t_adrf6720_settings;

#endif // ADRF6720_H
