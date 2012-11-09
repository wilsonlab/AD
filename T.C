void DrawButton2(char *string1, char *string2, char *string3,
				int x, int y, 
                int x2, int y2)
{
        drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
        drwfillbox(SET,(int)LIGHTGRAY,x+1,y+1,x2-1,y2-1);
        drwstring(SET,BLUE,string1,x+4,y+7);
        if (string2 != NULL) 
            drwstring(SET,YELLOW,string2,x+4,y+21);
        if (string3 != NULL) drwstring(SET,RED,string3,x+4,y+38);
        drwline(SET,WHITE,x+1,y+1,x2-1,y+1);
        drwline(SET,WHITE,x2-1,y+1,x2-1,y2-1);
        drwline(SET,BLACK,x+1,y+1,x+1,y2-1);
        drwline(SET,BLACK,x+1,y2-1,x2-1,y2-1);
}
