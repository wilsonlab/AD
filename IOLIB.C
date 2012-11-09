char **ReadHeader(fp,headersize)
FILE	*fp;
int	*headersize;
{
int	hasheader;
char	line[MAXLINE];
long	start;
char	**header_contents;
char	**new_header_contents;
int	nheaderlines;
int	done;
int	status;
int	eol;

    if(fp == NULL) return(NULL);
    if(headersize == NULL) return(NULL);
    hasheader = 1;
    nheaderlines = 0;
    header_contents = NULL;
    done = 0;
    /*
    ** determine the starting file position
    */
    start = ftell(fp);
    /*
    ** look for the magic start-of-header string
    */
    if(fread(line,sizeof(char),MAGIC_SOH_STRSIZE,fp) != MAGIC_SOH_STRSIZE){
	/*
	** unable to read the header
	*/
	hasheader = 0;
    } else {
	/*
	** null terminate the string
	*/
	line[MAGIC_SOH_STRSIZE-1] = '\0';
	/*
	** is it the magic start of header string?
	*/
	if((status = strcmp(line,MAGIC_SOH_STR)) != 0){
	    /*
	    ** not the magic string
	    */
	    hasheader = 0;
	} 
    }
    if(!hasheader){
	/*
	** no header was found so reset the file position to its starting
	** location
	*/
	fseek(fp,start,0L);
    } else
    /*
    ** read the header
    */
    while(!done && !feof(fp)){	
	/*
	** read in a line from the header
	*/
	if(fgets(line,MAXLINE,fp) == NULL){
	    /*
	    ** unable to read the header
	    */
	    fprintf(stderr,"ERROR in file header. Abnormal termination\n");
	    exit(-1);
	}
	/*
	** zap the CR
	*/
	if((eol = strlen(line)-1) >= 0){
	    line[eol] = '\0';
	}
	/*
	** look for the magic end-of-header string
	*/
	if(strcmp(line,MAGIC_EOH_STR) == 0){
	    /*
	    ** done
	    */
	    done = 1;
	} else {
	    /*
	    ** add the string to the list of header contents
	    ** by reallocating space for the header list
	    ** (dont forget the NULL entry at the end of
	    ** the list)
	    */
	    if(header_contents == NULL){
		if((header_contents = (char **)malloc(sizeof(char *)*2)) ==
		NULL){
		    fprintf(stderr,"initial malloc failed. Out of memory\n");
		    break;
		}
	    } else {
		if((new_header_contents = (char **)calloc(
		nheaderlines+2,sizeof(char *))) == NULL){
		    fprintf(stderr,"realloc failed. Out of memory\n");
		    break;
		}
		/*
		** copy the previous contents
		*/
		bcopy(header_contents,new_header_contents,sizeof(char
		*)*(nheaderlines +1));
		/*
		** and free the old stuff
		*/
		free(header_contents);
		/*
		** and reassign to the new stuff
		*/
		header_contents = new_header_contents;
#ifdef OLD
		if((header_contents = (char **)realloc(header_contents,
		sizeof(char *)*(nheaderlines+2))) == NULL){
		    fprintf(stderr,"realloc failed. Out of memory\n");
		    break;
		}
#endif
	    }
	    if((header_contents[nheaderlines] = 
	    (char *)malloc((strlen(line)+1)*sizeof(char))) == NULL){
		    fprintf(stderr,"malloc failed. Out of memory\n");
		    break;
	    }
	    strcpy(header_contents[nheaderlines],line);
	    header_contents[nheaderlines+1] = NULL;
	    nheaderlines++;
	}
    }
    /*
    ** report the headersize by comparing the current position with
    ** the starting position
    */
    *headersize = ftell(fp) - start;
    return(header_contents);
}

void DisplayHeader(fp,header,headersize)
FILE	*fp;
char	**header;
long	headersize;
{
int	i;

    if(fp == NULL) return;
    if(header != NULL){
	fprintf(fp,"Header size: \t%d bytes\n",headersize);
	fprintf(fp,"Header contents:\n");
	i = 0;
	while(header[i] != NULL){
	    fprintf(fp,"\t%s\n",header[i++]);
	}
    } else {
	fprintf(fp,"No header\n");
    }
}

/*
** returns the string value of a parameter imbedded in the header
*/
char *GetHeaderParameter(header,parm)
char	**header;
char	*parm;
{
int	i;
char	*value;
char	*ptr;
char	*strchr();

    value = NULL;
    if(header != NULL){
	/*
	** go through each line of the header
	*/
	for(i=0;header[i] != NULL;i++){
	    /*
	    ** search for the parameter string which must start on the
	    ** third character of the line
	    */
	    if(strlen(header[i]) < 3) continue;
	    /*
	    ** does it match
	    */
	    if(strncmp(header[i]+2,parm,strlen(parm)) == 0){
		/*
		** now return the value which begins following
		** the whitespace at the end of the parameter name
		*/
		for(value=header[i]+2+strlen(parm)+1;value,*value!='\0';value++){
		    /*
		    ** skip white space
		    */
		    if(*value != ' ' && *value != '\t' && 
		    *value != '\n'){
			/*
			** found the value and return it
			*/
			return(value);
		    }
		}
	    }
	}
    } 
    return(value);
}

