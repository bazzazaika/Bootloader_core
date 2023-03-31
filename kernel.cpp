// Эта инструкция обязательно должна быть первой, т.к. этот код компилируется в бинарный,
// и загрузчик передает управление по адресу первой инструкции бинарного образа ядра ОС.
__asm("jmp kmain");

#define VIDEO_BUF_PTR (0xb8000)
#define PIC1_PORT (0x20)
// Базовый порт управления курсором текстового экрана. Подходит для большинства, но может отличаться в других BIOS и в общем случае адрес должен быть прочитан из BIOS data area.
#define CURSOR_PORT (0x3D4)
#define VIDEO_WIDTH (80) // Ширина текстового экрана
#define IDT_TYPE_INTR (0x0E)
#define IDT_TYPE_TRAP (0x0F)
// Селектор секции кода, установленный загрузчиком ОС
#define GDT_CS (0x8)




//#define NULL ((char *)0)
#define MAX_LEN 41
#define LEN_COMMAND 11
#define LEN_TEMP 40
int mode;
char str;
char codes[57];
char big_codes[57];
//char buf[50];
int len = 0;
int numb_str = 0;

int cur_pos = 0;

char info[] = "info";
char upcase[] = "upcase";
char downcase[] = "downcase";
char titlize[] = "titlize";
char template_[] = "template";
char search[] = "search";
char shutdown[] = "shutdown";
char buf[MAX_LEN];
char temp[LEN_TEMP];
char temp_len[LEN_TEMP];




static inline unsigned char inb (unsigned short port) // Чтение из порта
{
	unsigned char data;
	asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
	return data;
}



static inline void outb (unsigned short port, unsigned char data) // Запись
{
	asm volatile ("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline void outw (unsigned short port, unsigned char data) // Запись
{
	asm volatile ("outw %w0, %w1" : : "a" (data), "Nd" (port));
}


// Функция переводит курсор на строку strnum (0 – самая верхняя) в позицию pos на этой строке (0 – самое левое положение).
void cursor_moveto(unsigned int strnum, unsigned int pos)
{
	unsigned short new_pos = (strnum * VIDEO_WIDTH) + pos;
	//cur_pos = new_pos;
	outb(CURSOR_PORT, 0x0F);
	outb(CURSOR_PORT + 1, (unsigned char)(new_pos & 0xFF));
	outb(CURSOR_PORT, 0x0E);
	outb(CURSOR_PORT + 1, (unsigned char)( (new_pos >> 8) & 0xFF));
	if (strnum >= 25)
	{
		unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	    int i;
	    for (i = 0; i < 25*80; i++)
	    {
	        video_buf[0] = 0;
	        video_buf += 2;
	    }
	    numb_str = 0;
	    cur_pos = 0;
	    cursor_moveto(numb_str, 0);
	}
}


void out_str(int color, const char* ptr, unsigned int strnum)
{
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 80*2 * strnum + cur_pos*2;
	while (*ptr)
	{
		video_buf[0] = (unsigned char) *ptr; // Символ (код)
		video_buf[1] = color; // Цвет символа и фона
		video_buf += 2;
		ptr++;
		cur_pos++;
	}
	cursor_moveto(strnum, cur_pos);
}


//print number
void my_print(int color, char ptr, unsigned int strnum)
{
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 80*2 * strnum + cur_pos*2;
	int i = 0;
	char string[3];
	int ind = 2;
	while(ptr> 0 && ind >= 0)
	{
		i = ptr%10;
		string[ind--] = i;
		ptr = ptr / 10;
	}
	while(ind<2)
	{
		if(string[ind] != 0)
		{
			break;
		}
		ind++;
	}
	while(ind<3)
	{
		
		video_buf[0] = string[ind]+'0'; // Символ (код)
		video_buf[1] = color; // Цвет символа и фона
		video_buf += 2;
		ind++;
		cur_pos++;
	}
	cursor_moveto(strnum, cur_pos);
}


void init_codes()
{
	codes[0] = 0;
	codes[1] = 0; //ESC
	codes[2] = '1';
	codes[3] = '2';
	codes[4] = '3';
	codes[5] = '4';
	codes[6] = '5';
	codes[7] = '6';
	codes[8] = '7';
	codes[9] = '8';
	codes[10] = '9';
	codes[11] = '0';
	codes[12] = '-';
	codes[13] = '=';
	codes[14] = 0; // backspace
	codes[15] = 0; // tab
	codes[16] = 'q';
	codes[17] = 'w';
	codes[18] = 'e';
	codes[19] = 'r';
	codes[20] = 't';
	codes[21] = 'y';
	codes[22] = 'u';
	codes[23] = 'i';
	codes[24] = 'o';
	codes[25] = 'p';
	codes[26] = '[';
	codes[27] = ']';
	codes[28] = 0; //enter
	codes[29] = 0; //l crtl
	codes[30] = 'a';
	codes[31] = 's';
	codes[32] = 'd';
	codes[33] = 'f';
	codes[34] = 'g';
	codes[35] = 'h';
	codes[36] = 'j';
	codes[37] = 'k';
	codes[38] = 'l';
	codes[39] = ';';
	codes[40] = '>';
	codes[41] = '+';
	codes[42] = 0;//l shift
	codes[43] = '\\';
	codes[44] = 'z';
	codes[45] = 'x';
	codes[46] = 'c';
	codes[47] = 'v';
	codes[48] = 'b';
	codes[49] = 'n';
	codes[50] = 'm';
	codes[51] = ',';
	codes[52] = '.';
	codes[53] = '/';
	codes[54] = 0;;//r shift
	codes[55] = 0;
	codes[56] = 0;//alt
	codes[57] = ' ';
}

void init_big_codes()
{
	big_codes[0] = 0;
	big_codes[1] = 0; //ESC
	big_codes[2] = '1';
	big_codes[3] = '2';
	big_codes[4] = '3';
	big_codes[5] = '4';
	big_codes[6] = '5';
	big_codes[7] = '6';
	big_codes[8] = '7';
	big_codes[9] = '8';
	big_codes[10] = '9';
	big_codes[11] = '0';
	big_codes[12] = '-';
	big_codes[13] = '=';
	big_codes[14] = 0; // backspace
	big_codes[15] = 0; // tab
	big_codes[16] = 'Q';
	big_codes[17] = 'W';
	big_codes[18] = 'E';
	big_codes[19] = 'R';
	big_codes[20] = 'T';
	big_codes[21] = 'Y';
	big_codes[22] = 'U';
	big_codes[23] = 'I';
	big_codes[24] = 'O';
	big_codes[25] = 'P';
	big_codes[26] = '[';
	big_codes[27] = ']';
	big_codes[28] = 0; //enter
	big_codes[29] = 0; //l crtl
	big_codes[30] = 'A';
	big_codes[31] = 'S';
	big_codes[32] = 'D';
	big_codes[33] = 'F';
	big_codes[34] = 'G';
	big_codes[35] = 'H';
	big_codes[36] = 'J';
	big_codes[37] = 'K';
	big_codes[38] = 'L';
	big_codes[39] = ';';
	big_codes[40] = '>';
	big_codes[41] = '+';
	big_codes[42] = 0;//l shift
	big_codes[43] = '\\';
	big_codes[44] = 'Z';
	big_codes[45] = 'X';
	big_codes[46] = 'C';
	big_codes[47] = 'V';
	big_codes[48] = 'B';
	big_codes[49] = 'N';
	big_codes[50] = 'M';
	big_codes[51] = ',';
	big_codes[52] = '.';
	big_codes[53] = '/';
	big_codes[54] = 0;;//r shift
	big_codes[55] = 0;
	big_codes[56] = 0;//alt
	big_codes[57] = ' ';
}

//print backspace
void print_backspace()
{
	
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	cur_pos--;
	if(cur_pos == -1)
	{
		cur_pos = 80;
		numb_str--;
	}
	video_buf += 80*2 * numb_str + 2 *(unsigned char)cur_pos;
	video_buf[0] = 0; // Символ (код)
	video_buf[1] = 0x07;
	cursor_moveto(numb_str, cur_pos);
}



//print 1 byte
void print(int color, const char ptr, unsigned int strnum)
{
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;

	video_buf += 80*2 * strnum + cur_pos*2;
	cur_pos++;
	if(cur_pos == 80)
	{
		cur_pos = 0;
		numb_str++;
	}
	video_buf[0] = (unsigned char) ptr; // Символ (код)
	video_buf[1] = color; // Цвет символа и фона
	cursor_moveto(numb_str, cur_pos);
}



int strcmp(char* str1, char* str2)
{
	char* s1 = str1;
	char* s2 = str2;
	while (*s2 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}
	if (*s1 == *s2)
		return 1;
	else
		return 0;
}

void print_error(char* str)
{
	cursor_moveto(++numb_str, cur_pos=0);
	out_str(0x07, str, numb_str);
	cursor_moveto(++numb_str, cur_pos=0);
}

void func_upcase(int ind, int max)
{
	
	if(ind >= max)
	{
		print_error("Error: no arguments");
		return;
	}
	cursor_moveto(++numb_str, cur_pos=0);
	while (ind <= max)
	{
		if(97 <= buf[ind] && buf[ind] <= 122)
		{
			print(0x07, buf[ind]-32, numb_str);
		}
		else
		{
			print(0x07, buf[ind], numb_str);
		}
		ind++;
	}
	cursor_moveto(++numb_str, cur_pos=0);
	
}

void func_downcase(int ind, int max)
{
	
	if(ind >= max)
	{
		print_error("Error: no arguments");
		return;
	}
	cursor_moveto(++numb_str, cur_pos=0);
	while (ind <= max)
	{
		if(65 <= buf[ind] && buf[ind] <= 89)
		{
			print(0x07, buf[ind]+32, numb_str);
		}
		else
		{
			print(0x07, buf[ind], numb_str);
		}
		ind++;
	}
	cursor_moveto(++numb_str, cur_pos=0);
	
}


void func_titlize(int ind, int max)
{
	if(ind >= max)
	{
		print_error("Error: no arguments");
		return;
	}
	cursor_moveto(++numb_str, cur_pos=0);
	char flag = 1;
	while (ind <= max)
	{
		if(97 <= buf[ind] && buf[ind] <= 122 && flag)
		{
			print(0x07, buf[ind]-32, numb_str);
			flag = 0;
		}
		else if(buf[ind] == ' ')
		{
			flag = 1;
			print(0x07, buf[ind]-32, numb_str);

		}
		else
		{
			print(0x07, buf[ind], numb_str);
			flag = 0;
		}
		ind++;
	}
	cursor_moveto(++numb_str, cur_pos=0);
}

void func_template(int ind, int max)
{
	len = 0;
	if(ind >= max)
	{
		print_error("Error: no arguments");
		return;
	}
	while (ind <= max && len<LEN_TEMP)
	{
		temp[len] = buf[ind];
		ind++;
		len++;
	}
	temp[len]=0;
	if(mode == 0)
	{
		cursor_moveto(++numb_str,cur_pos = 0);
		out_str(0x007, "Template '", numb_str);
		cursor_moveto(numb_str,cur_pos);
		out_str(0x007, temp, numb_str);
		cur_pos--;
		out_str(0x007, "' loaded.", numb_str);
		cursor_moveto(++numb_str,cur_pos = 0);
	}
	else
	{
		cursor_moveto(++numb_str,cur_pos = 0);
		out_str(0x007, "Template '", numb_str);
		out_str(0x007, temp, numb_str);
		cur_pos--;
		out_str(0x007, "' loaded. BM info:", numb_str);
		cursor_moveto(++numb_str,cur_pos = 0);
		int i = len - 3;
		int j = len - 3;
		char flag =0;
		while(i>=0)
		{
			
			while(j != i)
			{
				if(temp[i] == temp[j])
				{
					flag = 1;
					temp_len[i] = temp_len[j];
					break;
				}
				j--;
			}
			j = len-3;
			if(!flag)
			{
				
				temp_len[i] = len  - i-2;
			}
			flag =0;
			i--;

		}
		i = len - 2;
		j = len -3;
		flag = 0;
		while(j >=0)
		{
			if(temp[i] == temp[j])
			{	
				flag = 1;
				temp_len[i] = temp_len[j];
				break;
			}
			j--;
		}
		if(!flag)
		{
			temp_len[i] = len-2;
		}


		for(i = 0; i < len - 1; i++ )
		{
			print(0x007,temp[i], numb_str);
			out_str(0x007, ":", numb_str);
			print(0x007,temp_len[i] + '0', numb_str);
			out_str(0x007, " ", numb_str);
			
		}
		cursor_moveto(++numb_str,cur_pos=0);
	}
}

int memcmp (char *a_,char *b_, int size) 
{
  char *a = a_;
  char *b = b_;
  for (; size-- > 0; a++, b++)
    if (*a != *b)
      return *a > *b ? +1 : -1;
  return 0;
}

static int suffix_match(char* needle, int nlen, int offset, int suffixlen)
{
    if (offset > suffixlen)
        return needle[offset - suffixlen - 1] != needle[nlen - suffixlen - 1] &&
        memcmp(needle + nlen - suffixlen, needle + offset - suffixlen, suffixlen) == 0;
    else
        return memcmp(needle + nlen - offset, needle, offset) == 0;
}

static int max(int a, int b)
{
    return a > b ? a : b;
}


int bm (char* haystack, int hlen, char* needle, int nlen)
{
    char skip[40];
    char occ[256];

    if (nlen > hlen || nlen <= 0 || !haystack || !needle)
        return 0;

    for (int a = 0; a < 255 + 1; ++a)
        occ[a] = -1;

    for (int a = 0; a < nlen - 1; ++a)
        occ[needle[a]] = a;
    for (int a = 0; a < nlen; ++a)
    {
        int offs = nlen;
        while (offs && !suffix_match(needle, nlen, offs, a))
            --offs;
        skip[nlen - a - 1] = nlen - offs;
    }

    for (int hpos = 0; hpos <= hlen - nlen; )
    {
        int npos = nlen - 1;
        while (needle[npos] == haystack[npos + hpos])
        {
            if (npos == 0)
                return hpos;

            --npos;
        }
        hpos += max(skip[npos], npos - occ[haystack[npos + hpos]]);
    }
    return -1;
}


void func_search(int ind,int max)
{
	cursor_moveto(++numb_str,cur_pos=0);
	char str[40];
	int i =0;
	while (i+ind<=max)
	{
		str[i] = buf[i+ind];
		i++;
	}
	str[i] = 0;
	int sourcelen = i - 1;
	len--;
	if(len>sourcelen || len == 0)
	{
		out_str(0x007, "Not found'", numb_str);
		cursor_moveto(numb_str,cur_pos);
		out_str(0x007, temp, numb_str);
		cursor_moveto(numb_str,cur_pos);
		cur_pos--;
		out_str(0x007, "'", numb_str);
		cursor_moveto(numb_str,cur_pos);
		cursor_moveto(++numb_str,cur_pos=0);
		return;
	}
	int c = bm(str, sourcelen, temp,len);
	if(c != -1)
	{
		cursor_moveto(numb_str,cur_pos);
		out_str(0x007, "Found '", numb_str);
		out_str(0x007, temp, numb_str);
		cur_pos--;
		out_str(0x007, "' at pos:", numb_str);
		my_print(0x007, c, numb_str);
	}
	else
	{
		out_str(0x007, "Not found '", numb_str);
		cursor_moveto(numb_str,cur_pos);
		out_str(0x007, temp, numb_str);
		cursor_moveto(numb_str,cur_pos);
		cur_pos--;
		out_str(0x007, "'", numb_str);
		cursor_moveto(numb_str,cur_pos);
	}
	
	cursor_moveto(++numb_str,cur_pos=0);
}




void func_shutdown()
{
	asm volatile ("outw %w0, %w1" : : "a" (0x2000), "Nd" (0x604));
}



void parser()
{
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf+= 80*2*numb_str;
	int ind = 0;
	while(ind<=cur_pos)
	{
		buf[ind++] = *video_buf;
		video_buf+=2;
	}
	buf[ind] = 0;
	ind = 0;
	while(buf[ind] == ' ' && ind++ <=cur_pos);
	char command[LEN_COMMAND];
	int length = 0;
	while((buf[ind] != ' ') && (ind < cur_pos) && (length < LEN_COMMAND))
	{
		command[length] = buf[ind];
		ind++;
		length++;
	}
	if(length >=11)
	{
		print_error("Error: too big command.");
		return;

	}
	command[length] = 0;
	while(buf[ind] == ' ' && ind++ <=cur_pos);
	if(strcmp(command, info))
	{
		cursor_moveto(++numb_str, cur_pos=0);
 		cur_pos =0;
		out_str(0x07, "StringOS, Oleg Vasiliev 4851003/10001", numb_str);
		cursor_moveto(++numb_str, cur_pos=0);
		cur_pos =0;
		out_str(0x07, "OS: Linux; GNU assembler; syntax: AT&T; comp: gcc", numb_str);
		cursor_moveto(++numb_str, cur_pos=0);
		if (mode == 0)
		{
			cur_pos =0;
			out_str(0x07, "std mode", numb_str);
			cursor_moveto(++numb_str, cur_pos=0);
		}
		else
		{
			cur_pos =0;
			out_str(0x07, "bm mode", numb_str);
			cursor_moveto(++numb_str, cur_pos=0);
		}
	}
	else if(strcmp(command, upcase))
	{ 
		func_upcase(ind,cur_pos);
	}
	else if(strcmp(command, downcase))
	{ 
		func_downcase(ind,cur_pos);
	}
	else if(strcmp(command, titlize))
	{ 
		func_titlize(ind,cur_pos);
	}
	else if(strcmp(command, template_))
	{ 
		//while(buf[ind] == ' ' && ind++ <=cur_pos);
		func_template(ind,cur_pos);
		
	}
	else if(strcmp(command, search))
	{ 
		func_search(ind,cur_pos);

	}
	else if(strcmp(command, shutdown))
	{

		func_shutdown();
		cur_pos =0;
		cursor_moveto(++numb_str, cur_pos=0);
	}
	else 
	{ 
		if (cur_pos == 0)
		{
			cursor_moveto(++numb_str, cur_pos=0);
		}
		else
		{
			print_error("Error: unknown commands");
			
		}
		
	}
 		
}


// Структура описывает данные об обработчике прерывания
struct idt_entry
{
	unsigned short base_lo; // Младшие биты адреса обработчика
	unsigned short segm_sel; // Селектор сегмента кода
	unsigned char always0; // Этот байт всегда 0
	unsigned char flags; // Флаги тип. Флаги: P, DPL, Типы - это константы - IDT_TYPE...
	unsigned short base_hi; // Старшие биты адреса обработчика
} __attribute__((packed)); // Выравнивание запрещено


// Структура, адрес которой передается как аргумент команды lidt
struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed)); // Выравнивание запрещено

struct idt_entry g_idt[256]; // Реальная таблица IDT
struct idt_ptr g_idtp; // Описатель таблицы для команды lidt

// Пустой обработчик прерываний. Другие обработчики могут быть реализованы по этому шаблону
void default_intr_handler()
{
	asm("pusha");
	// ... (реализация обработки)
	asm("popa; leave; iret");
}

typedef void (*intr_handler)();

void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr)
{
	unsigned int hndlr_addr = (unsigned int) hndlr;
	g_idt[num].base_lo = (unsigned short) (hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short) (hndlr_addr >> 16);
}

// Функция инициализации системы прерываний: заполнение массива с адресами обработчиков
void intr_init()
{
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	for(i = 0; i < idt_count; i++)
		intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR, 
			default_intr_handler); // segm_sel=0x8, P=1, DPL=0, Type=Intr
}

void intr_start()
{
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	g_idtp.base = (unsigned int) (&g_idt[0]);
	g_idtp.limit = (sizeof (struct idt_entry) * idt_count) - 1;
	asm("lidt %0" : : "m" (g_idtp) );
}

void intr_enable()
{
	asm("sti");
}

void intr_disable()
{
	asm("cli");
}











char mode_big_code =0;
void on_key(unsigned char scan_code)
{

	if (scan_code == 14) //backspase
	{
		//if(cur_pos+numb_str*80 > 0)
		if(cur_pos > 0)
		{
			print_backspace();
			return;
		}
		else
		{
			return;
		}
	}
	if (scan_code == 28)
	{
		parser();
		return;
	}
	if (scan_code == 42)
	{
		mode_big_code = 1;
		return;
	}
	if(cur_pos <= 40)
	{
		if(mode_big_code)
		{
			print(0x07, big_codes[scan_code], numb_str);
			mode_big_code = 0;
			return;
		}
		print(0x07, codes[scan_code], numb_str);
	}
}










void keyb_process_keys()
{
	// Проверка что буфер PS/2 клавиатуры не пуст (младший бит присутствует)
	if (inb(0x64) & 0x01)
	{
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60); // Считывание символа с PS/2 клавиатуры
		if (scan_code < 128) // Скан-коды выше 128 - это отпускание клавиши
			on_key(scan_code);
	}
}

void keyb_handler()
{
	asm("pusha");
	// Обработка поступивших данных
	keyb_process_keys();
	// Отправка контроллеру 8259 нотификации о том, что прерывание обработано
	outb(PIC1_PORT, 0x20);
	asm("popa; leave; iret");
}

void keyb_init()
{
	// Регистрация обработчика прерывания
	intr_reg_handler(0x09, GDT_CS, 0x80 | IDT_TYPE_INTR, keyb_handler); // segm_sel=0x8, P=1, DPL=0, Type=Intr
	// Разрешение только прерываний клавиатуры от контроллера 8259
	outb(PIC1_PORT + 1, 0xFF ^ 0x02); // 0xFF - все прерывания, 0x02 - бит IRQ1 (клавиатура).
	// Разрешены будут только прерывания, чьи биты установлены в 0
}









const char* g_test = "";


extern "C" int kmain()
{
	
	asm ("movb %cl, str");
	
	
	if (str == 'a')
	{
		out_str(0x07, "std mode", numb_str++);
		mode = 0;	}
	else if (str == 'b')
	{
		out_str(0x07, "bm mode", numb_str++);
		mode = 1;
	}
	else
	{
		out_str(0x07, "error", numb_str++);
	}
	const char* hello = "Welcome to StringOS!";
	// Вывод строки
	cursor_moveto(numb_str, cur_pos = 0);
	out_str(0x07, hello, numb_str++);
	//out_str(0x07, g_test, numb_str++);
	cursor_moveto(numb_str, cur_pos = 0);


	intr_disable();

	intr_start();
	intr_init();
	init_codes();
	init_big_codes();
	keyb_init();
	

	intr_enable();


	// Бесконечный цикл
	while(1)
	{
		asm("hlt");
	}
	return 0;
}