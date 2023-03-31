    .att_syntax
    .code16
    #.section .text

    .global _start
_start:
# Инициализация адресов сегментов. Эти операции требуется не для любого BIOS, но их рекомендуется проводить.
    movw %cs, %ax
    #movw $0, %ax # Сохранение адреса сегмента кода в ax
    movw %ax, %ds # Сохранение этого адреса как начало сегмента данных
    movw %ax, %ss# И сегмента стека
    #movw %ax, %es
    movw _start,%sp # Сохранение адреса стека как адрес первой инструкции этого кода. Стек будет расти вверх и не перекроет код.
    
    #call puts
    
    movb $0x0, %ah
    movb $0x03, %al
    int $0x10
    
    movw $loading_str, %bx
    call puts
    jmp JUMP
    loading_str: .asciz "StringOS: "
    std_: .ascii "std\0"
    bm_: .ascii "bm\0"
JUMP:
    movw $0x0003, %ax   
    /*
    movb $0x0e, %ah  # В ah номер функции BIOS: 0x0e - вывод символа на активную видео страницу (эмуляция телетайпа)
    movb $'H', %al # В al помещается код символа
    int $0x10 # Вызывается прерывание. Обработчиком является код BIOS. Символ будет выведен на экран.
    movb $'e',%al
    int $0x10
    movb $'l', %al
    int $0x10
    #int $0x10
    movb $'o', %al 
    int $0x10
    */
    
    

call_kernel:
    #movb %ax, 0x1fd
    
    
    
          
    movw $0x1000, %ax
    movw %ax, %es
    
    movb $0x02, %ah #read number of sectors from disk to mem
    movb $0x01, %dl #disk
    movb $0x00, %dh #golovka
    movb $0x00, %ch #cilindr
    movb $0x01, %cl #sector number
    movb $0x12, %al #number of sectors
    movw $0x00, %bx
    int $0x13
    
    movb $0x02, %ah #read number of sectors from disk to mem
    movb $0x01, %dl #disk
    movb $0x01, %dh #golovka
    movb $0x00, %ch #cilindr
    movb $0x01, %cl #sector number
    movb $0x12, %al #number of sectors
    movw $0x2400, %bx
    int $0x13
    
    movb $0x02, %ah #read number of sectors from disk to mem
    movb $0x01, %dl #disk
    movb $0x01, %dh #golovka
    movb $0x00, %ch #cilindr
    movb $0x02, %cl #sector number
    movb $0x12, %al #number of sectors
    movw $0x4800, %bx
    int $0x13
     # Отключение прерываний
    
        #____1
read_cl:
    movb $0x0, %ah
    int $0x16
    
    movb $0x0e, %ah
    int $0x10
comp:   
    cmp $'s', %al 
    je comp_s
    
    cmp $'b', %al
    je comp_b
    
    jmp read_cl
    
    
        #____2
comp_s:
    movb $0x0, %ah
    int $0x16
    
    movb $0x0e, %ah
    int $0x10
    
    cmp $'t', %al 
    jne comp
    
    movb $0x0, %ah
    int $0x16
    
    movb $0x0e, %ah
    int $0x10
    
    cmp $'d', %al 
    jne comp 
 movb $'a', %cl   
    jmp switch
    
    
        #____3
comp_b:
    movb $0x0, %ah
    int $0x16
    
    movb $0x0e, %ah
    int $0x10
    
    cmp $'m',%al 
    jne comp
movb $'b', %cl
    
switch:
    /*
    movb $0x0e, %ah
    movb %cl, %al
    int $0x10
    */
    
    movb $0x0, %ah
    movb $0x03, %al
    int $0x10
    
            
    cli
    # Загрузка размера и адреса таблицы дескрипторов
    lgdt gdt_info
    # Включение адресной линии А20
    inb $0x92, %al
    orb $2, %al
    outb %al, $0x92
    # Установка бита PE регистра CR0 - процессор перейдет в защищенный режим
    movl %cr0, %eax
    orb $1, %al
    movl %eax, %cr0
    ljmp $0x8, $protected_mode # "Дальний" переход для загрузки корректной информации в cs, архитектурные особенности не позволяют этого сделать напрямую).

puts:
    movb 0(%bx), %al
    test %al, %al
    jz end_puts
    movb $0x0e, %ah
    int $0x10
    addw $1, %bx
    jmp puts
end_puts:
    ret
    
inf_loop:
    jmp inf_loop # Бесконечный цикл

gdt:
    #Нулевой дескриптор
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    /*
    Сегмент кода: base=0, size=4Gb, P=1, DPL=0, S=1(user),
    Type=1(code), Access=00A, G=1, B=32bit
    */
    .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
    /*
    Сегмент данных: base=0, size=4Gb, P=1, DPL=0, S=1(user),
    Type=0(data), Access=0W0, G=1, B=32bit
    */
    .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
    
    
gdt_info:
#Данные о таблице GDT (размер, положение в памяти)
    .word gdt_info - gdt #Размер таблицы (2 байта)
    .word gdt, 0 #32-битный физический адрес таблицы.



.code32
protected_mode:
# Здесь идут первые инструкции в защищенном режиме
    #Загрузка селекторов сегментов для стека и данных в регистры
    movw $0x10, %ax #Используется дескриптор с номером 2 в GDT
    movw %ax, %es
    movw %ax, %ds
    movw %ax, %ss
    #Передача управления загруженному ядру
    call 0x10000 #Адрес равен адресу загрузки в случае если ядро скомпилировано в "плоский" код




# Внимание! Сектор будет считаться загрузочным, если содержит в конце своих 512 байтов два следующих байта: 0x55 и 0xAA
    .fill 0x1fe - (. - _start) ,1,0 #510
    
    .word 0xaa55 # 2 необходимых байта чтобы сектор считался загрузочным
    