#ifndef LCD_VDG_H
#define LCD_VDG_H

#define Black 0
#define Red 63488
#define Green 2016
#define Blue 31
#define White 65535
#define Purple 61727
#define Yellow 65504
#define Cyan 047
#define d_gray 21130
#define l_gray 31727

//----------------------------------------------------------------
// General Functions
//----------------------------------------------------------------
extern int fabs(int val);  
extern int TCHVerif(void);
extern int commVDG(unsigned char *vparam);

//----------------------------------------------------------------
// LCDG Basic Graphic Functions
//----------------------------------------------------------------
extern void Write_Command(unsigned int wcommand);
extern void Write_Data(unsigned int wdata);
extern unsigned int Read_Data(void);
extern void Write_Command_Data(unsigned int wcommand,unsigned int Wdata);
extern void TFT_Set_Address(unsigned int PX1,unsigned int PY1,unsigned int PX2,unsigned int PY2);
extern void TFT_Init(void);
extern unsigned int Set_Color(unsigned int R,unsigned int G,unsigned int B);

//----------------------------------------------------------------
// LCDG Advanced Graphic Functions
//----------------------------------------------------------------
extern void TFT_Fill(unsigned int color);
extern void TFT_Box(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color);
extern void TFT_Dot(unsigned int x,unsigned int y,unsigned int color);
extern void TFT_Line(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color);
extern void TFT_H_Line(char X1,char X2,unsigned int y_pos,unsigned int color);
extern void TFT_V_Line(unsigned int Y1,unsigned int Y2,char x_pos,unsigned int color);
extern void TFT_Rectangle(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color);
extern void TFT_Circle(unsigned int x,unsigned int y,char radius,char fill,unsigned int color);
extern void TFT_Char(char C,unsigned int x,unsigned int y,char DimFont,unsigned int Fcolor,unsigned int Bcolor);
extern void TFT_Text(char* S,unsigned int x,unsigned int y,char DimFont,unsigned int Fcolor,unsigned int Bcolor);
extern void TFT_Image(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y,unsigned int *picture);
extern void TFT_InvertRect(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y);
extern void TFT_Scroll(unsigned char qtdlin, unsigned char ptipo);
extern void TFT_SaveScreen(unsigned int paddress, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
extern void TFT_RestoreScreen(unsigned int paddress, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

//----------------------------------------------------------------
// Keyboard Functions
//----------------------------------------------------------------
extern void ShowKeyboard(unsigned int xposini, unsigned int yposini, unsigned char vnewkey);
extern void HideKeyboard(unsigned int xposini, unsigned int yposini);
extern void ReturnKeyboard(unsigned int xposini, unsigned int yposini, unsigned int vpostx, unsigned int vposty);

#endif /* LCD_VDG_H */
