/* 	converter.c	(C)	Mithril Software 1985

	This program converts pictures from 8-bits/byte-dump to
	suitable control sequences for the FX-100 printer, writes
	them to a file if orderd to, opens /dev/lp and print them.

	The program is started by execl from /usr/bin/paint.
	Standard input is connected to a pipe for communication
	with paint.

        MODIFIED:       November      1985
                                Swedish or English texts selectable
				for messages.

*/

#include <stdio.h>
#include <fcntl.h>

char io_buffer[1024],filename[30],buff[80];
int io_len=0,expand=0,slask,fd,xmax,ymax,language;

/*
	Write data to file.
*/
print(bu,bulen)
char *bu[];
int bulen;
{
    write(fd,bu,bulen);
}


/*
	Takes care of internal buffering and expansion of lines.
*/
putbyte(c)
char c;
{
    char buff[5];
    int i;

    if(c==0xFF){
        if(expand) for(i=0;i<128;i++) io_buffer[io_len++]=0;
        io_buffer[io_len++]='\n';
        print(io_buffer,io_len);
        io_len=0;
    }
    else{
        io_buffer[io_len++]=c;
    }
}

/*
	Transforms a picture wider than 600 pixels.
*/
l_transform()
{
    register unsigned char bytes;
    register short pos,nr,line,line_length,count,nr_bytes;

    union area {
	unsigned char  c[8];
	unsigned int   i[2];
    } *data,w;

    /*
    	Calculate size of workarea and allocate it.
    */
    nr_bytes=(xmax+7)/8;
    line_length=(nr_bytes+6)/7;
    data=(union area *)malloc(sizeof(union area)*line_length*ymax);

    /*
    	Main loop - transform the dump to needle matrix.
    */
    for(line=0;line<ymax;line++){
	pos=line*line_length;
	count=0;
        if(nr_bytes>7){
	    nr=7;
	}
	else{
	    nr=nr_bytes;
	}
        do{ w.i[0]=w.i[1]=0;

            read(0,&w,nr);
	    count+=nr;
            for(bytes=0;bytes<8;bytes++){
	        w.i[1]>>=1;
                if(w.c[3]&0x01) w.c[4]|=0x80;
		w.i[0]>>=1;
		data[pos].c[bytes]=(w.c[bytes]&0x7F);
	    }
            pos++;
            if((nr=nr_bytes-count)>7)nr=7;
	}while(nr>0);
    }

    /*
    	Write it.
    */
    for(count=0;count<line_length;count++){
        for(bytes=0;bytes<8;bytes++){
            print(buff,5);
            for(pos=count;pos<(ymax*line_length+count);pos+=line_length)
	        putbyte(data[pos].c[bytes]);
            putbyte(0xFF);
	}
    }
    free(data);
}


/*
	Transform a picture not wider than 600 pixels.
*/
p_transform()
{
    register unsigned char out;
    register short tmp,index,line_index,out_count,byte_count,line,nr;
    register short lines,bytes,pels,indata_size;
    unsigned char *outdata[200],*indata;

    /*
    	Calculate size of work area and allocate it.
    */
    lines=(ymax+6)/7;
    bytes=(xmax+7)/8;
    pels=bytes*8;
    indata_size=bytes*7;
    for(index=0;index<lines;index++)
        outdata[index]=(unsigned char *)malloc(pels);
    indata=(unsigned char *)malloc(indata_size);
    line=lines-1;

    /*
    	The main loop - converts a picture to printer format.
    */
    do{
	nr=read(0,indata,indata_size);
	if(nr<indata_size){
	    for(index=nr;index<indata_size;index++)indata[index]=0;
	}
        out_count=0;
        for(index=0;index<bytes;index++){
           for(byte_count=0;byte_count<8;byte_count++){
	       out=0;
	       for(line_index=0;line_index<indata_size;line_index+=bytes){
		   tmp=line_index+index;
		   if(indata[tmp]&0x80)out|=0x80;
		   out>>=1;
		   indata[tmp]<<=1;
	       }
	       outdata[line][out_count++]=out;
	   }
	}
	line--;
	out_count=0;
    }while(nr==indata_size  && line>=0);

    /*
    	Print the picture.
    */
    for(line_index=0;line_index<lines;line_index++){
        print(buff,5);
	for(index=0;index<pels;index++) putbyte(outdata[line_index][index]);
        free(outdata[line_index]);
	putbyte(0xFF);
    }
}

main(argc,argv)
int argc;
char *argv[];
{
    char *l,*getenv();
    int language;

    setpgrp();

    if(!(l=getenv("LANGUAGE")))l="english";

    if(strcmp(l,"english")) language=0; else language=1;

    if(argc<4){
	if(language)
	    fprintf(stderr,"usage: converter xmax ymax file < dump\n");
	else
	    fprintf(stderr,"usage: converter xmax ymax fil < dump\n");
	exit(-1);
    }

    xmax=atoi(*++argv);
    ymax=atoi(*++argv);
    if(xmax<42 || ymax<42){
	if(language){
	fprintf(stderr,"converter: no smaller pictures than 42x42 please.\n");
	}
	else{
    	fprintf(stderr,"converter: ej mindre bilder {n 42x42 tack.\n");
	}
	exit(-1);
    }

    strcpy(filename,*++argv);
    /*
        Open outfile.
    */
    if ((fd=open(filename,O_WRONLY | O_CREAT))<0){
        if(language)
	    fprintf(stderr,"converter: can't creat %s\n",*argv);
	else
	    fprintf(stderr,"converter: kan ej skapa %s\n",*argv);
        exit(-1);
        }
    /*
	Initiate printer for 7 pixels/LF
    */
    print("\033@\033A\7",5);

    /*
	Select transformation procedure
    */
    if(xmax<=600){
        slask=(((xmax+7)/8)<<3);
	expand=slask%256 > 127;
	sprintf(buff,"\33*\5%c%c",
	expand ? slask%128 : slask%256,
	expand ? slask/256+1 : slask/256);
	p_transform();
	}
	else{
            expand=ymax%256 > 127;
	    sprintf(buff,"\33*\5%c%c",
  	    expand ? ymax%128 : ymax%256,
	    expand ? ymax/256+1 : ymax/256);
	    l_transform();
	}

    /*
	Close file.
    */
    print("\033@",2);
    close(fd);
}
