ACOMPAL MDD Editor v1.00 Std
=============================================
  /**************************
 /*  Menu item description
/*****************************
File:
Load MDD: Open a MDD file.
Export Bmp:Export datas to a bitmap file.
Import Bmp:Import a bitmap file, 336 * 240, 24bit.
Check Info Save As: Choose anther info file to continue save Pins on it.The default is filename + ".txt",appeared in files's folder.
Exit: Quit MDD Editor.

View:
Cursor Color: Choose a color to make the cursor more readble.
Clip Mode: the 320*224 or 256 * 224 is the same to Genesis.

Tools:
Export to Functon: Export the datas to C Style function(stored in a head file).
Test LDU: With these options,you can test the complied LDU more easily.

LEFT,RIGHT,UP,DOWN:
Move the cursor.

About:
Some information about this soft.




  /**************************
 /*  How to complie a LDU.
/*****************************

In order to compile a LDU,thiese tools are required.

1) Gens32 Surreal v1.28 LDU+
2) MDD Editor(this)
3) C\C++ complier
4) Dictionary.
5) The rom.
6) A soft to edit images.

Ok,After gotted these tools,let's start the work now.

/////////////////////////////////////////////////////////////////////////////////
First step: collect texts.

In this step,we need to play the game with Gens32.When we found the text,
press short cut(shift+L) to save the screen to a mdd file.

/////////////////////////////////////////////////////////////////////////////////
Second step: Convert mdd to bitmap.

In this step,open mdd with MDD Editor first,then select File->Export Bmp.
choose a good filename is recommended.

/////////////////////////////////////////////////////////////////////////////////
Third step: translate the texts in bitmap,and convert them to C style functions.

Open these bitmaps with image editor,replace the text with translated words.After
that,open the bitmap with MDD Editor(File->Import Bmp).Select "tools->Export to function".
This will convert the datas to a head file. Notice: the function name must in C style,That
means,no spaces,no strange characters,and can not start with a figure.

/////////////////////////////////////////////////////////////////////////////////
Fourth Step: Comple the LDU.

Download a LDU modle.copy the head files you gotted in last setp to the modle's folder(I recommend
that rewrite the functions,or it will got you headache later).open the project.Open the main.cpp.
found this mark:

//TD Add Checking Code here;
/*************************/

You will add the codes here.




  /**************************
 /*  How to write the codes.
/*****************************

Step 1: Choose Pins.

Open MDD with MDD editor,left click to choose a check point(we called this 'Pin').Check out the "UNUSED"
symbol down right,right click to save the Pin to a info file.Since we will based on these Pins to identify the
Screen,please choose them carefully.

//////////////////////////////////////////////////////////////////////////////////
Step 2: Write codes.
The simplest action is just check and display.Here is how to implement this(the simplest,but not the most usefull.If you
are a skilled programmer,please find the best way out).

 if(Pin[71107] == 65535 //These are the Pins you selected in MDD,you can find them in info file.
 && Pin[69091] == 65535
 && Pin[67747] == 65535
 && Pin[71107] == 65535
 && Pin[68755] == 64640
 && Pin[68755] == 64640)
{
	Text_Out_THE_SAMPLE(Pin); //This is the function you exported form bmp.In order to use it,you must add some words top of
				  //The file.The words is #include "Text_Sample.h" .Here,the 'Text_Sample.h' is the head file name you
				  //Exported.
};

Ok,after finshed the codes,just press Ctr + Shift + B(VC++ .net) to complie it,enjoy!.
