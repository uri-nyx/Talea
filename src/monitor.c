/* FRONTEND FOR TALEA VM */
#include <stdio.h>
#include <stdlib.h>
#include "lib/talea.h"
#include "lib/termbox/termbox.h"
#include "tui/tui.h"

//100x41 characters
int buff_count;
int comm = 0;

/// COMMAND DEFINITIONS
void set_mem(char* c) {
    char addr[5];
    char val[3];

    addr[0] = c[2];
    addr[1] = c[3];
    addr[2] = c[4];
    addr[3] = c[5];
    addr[4] = '\0';

    val[0] = c[7];
    val[1] = c[8];
    val[2] = '\0';

    int a = strtol(addr, NULL, 16);
    int v = strtol(val, NULL, 16);;

    memory[(word_t)a] = (byte_t)v;
}

void com(uint32_t* buff) {
    char c[256];

    
    //printf(" c: %s -- ", c);

    int in = 0;
    while (buff[in] != '\0')
    {
        tb_utf8_unicode_to_char(&c[in], buff[in]);
        in++;
    }
    
    //printf(" buff: %s -- ", c);

    switch (c[0])
    {
    case 'm':
        set_mem(c);
        break;
    
    default:
        break;
    }

    for (size_t i = 0; i < 256; i++)
    {
        c[i] = '\0';
    }
    
    
    
    
}

void render_widget(int x, int y, char* buff) {
    int i = 0;
    while (buff[i] != '\0')
    {
        if (buff[i] == ' '){
            x++;
            i++;
        } else {
            tb_set_cell(x, y, (uint32_t)buff[i], 23, 2);
            x++;
            i++;
        }
    }
}

void render_flags(uint8_t flags){
    tb_printf(flags_w.x, flags_w.y + 1, 15, 2, " n v - - - - z c  val: %x", regs.status);
    int x = flags_w.x;

    for (size_t i = 0; i < 8; i++)
    {
        x++;
        (flags >> (7 - i)) & 1 ? tb_print(x, flags_w.y, 23, 2, "○") : 
                                    tb_print(x, flags_w.y, 9, 2, "○");
        x++;  
        
    }
}

void actualize_memory(){
    for (size_t i = 0; i < 32; i++)
    {
        for (uint16_t j = 0; j < 16; j++)
        {
            memory_w[i][j] = memory[(memory_w_row[i] << 4) | j];
        }
        
    }
    
}

void render_memory(){
    int x = 2;
    int y = 7;
    actualize_memory();

    for (size_t i = 0; i < 32; i++)
    {
        x = 2; 
        tb_printf(x, y, 23, 2, "%.3x", memory_w_row[i]);

        x = 9;
        for (size_t j = 0; j < 16; j++)
        {
            tb_printf(x, y, 23, 2, "%.2x", memory_w[i][j]);
            x += 3;
        } 

        y++;       
    }  
}

void render_stack(){

    int x = 60;
    int y = 7;

    for (size_t i = 0; i < 32; i++)
    { 
        tb_printf(x, y, 23, 2, "%.2x", st.stack[i]);
        y++;       
    }
    tb_set_cell(x, 7 + st.pointer, *st.stack[st.pointer], 0, 23); 

} 

void render_layout(){
        for (int i = 0; i < 44; i++) 
    {
        tb_print(0, i, 15, 2, tui[i]);
    }

};

void render_pc_cursor(uint16_t pc) {
    uint16_t pos = pc & 0x1ff;
    int x = ((pos & 0xf) * 3) + 9; //last nibble offset
    int y = (pos >> 4) + 7; // 1 byte

    if (((pc >> 4) > (memory_w_row[0] - 1)) && ((pc >> 4) < (memory_w_row[31])))
    {
        tb_printf(x, y, 0, 23, "%.2x", memory[pc]);
    }
    
}


void memory_w_scroll_down(){

    if (memory_w_row[0] < 0xfff)
    {
        for (size_t i = 0; i < 32; i++)
        {
            memory_w_row[i]++;
                for (size_t j = 0; j < 16; j++)
                {
                    memory_w[i][j] = memory[((memory_w_row[i] + 1) << 4) | j];
                }
                            
        }
        
    }
    
}

void memory_w_scroll_up(){

    if (memory_w_row[0] > 0) {
    for (size_t i = 0; i < 32; i++)
    {
            memory_w_row[i]--;
                for (size_t j = 0; j < 16; j++)
                {
                    memory_w[i - 1][j] = memory[((memory_w_row[i] - 1) << 4) | j];
                }   
             
    }
   
    }
}

void track_pc_cursor(uint16_t pc) {

    int y = pc >> 4;

    if (y > memory_w_row[31])
    {
        while (y > memory_w_row[31])
        {
            memory_w_scroll_down();
        }
    } else if ( y < memory_w_row[0]) {

        while (y < memory_w_row[0])
        {
            memory_w_scroll_down();
        }
    }
    
    render_pc_cursor(regs.pc);
    

}

void get_command(uint32_t* buff, widg wb){
    struct tb_event input;
    
    tb_print(wb.x, wb.y, 23, 2, "> ");
    tb_present();
    wb.x += 2;
    int start_comm = wb.x;
    tb_set_cursor(wb.x, wb.y);
    
    while (1) 
    {   
        tb_poll_event(&input);

        if (input.key == TB_KEY_BACKSPACE2) {
            if (wb.x > start_comm)
            {
                buff[buff_count] = (uint32_t)' '; 
                tb_set_cell(wb.x - 1, wb.y, (uint32_t)' ', 23, 2);
                tb_set_cursor(wb.x - 1, wb.y);
                tb_present();
                wb.x--;
                buff_count--;
                continue;
                
            }
        } else if (input.key == TB_KEY_ENTER) {
            com(buff);
            tb_present();
            
            wb.x = 2;
            break;
        } else if (input.ch) {
            buff[buff_count] = input.ch;
            tb_set_cell(wb.x, wb.y, (uint32_t)input.ch, 23, 2);
            tb_set_cursor(wb.x + 1, wb.y);
            tb_present();
            wb.x++;
            buff_count++;

        }
       
    }
    
    
    for (size_t i = 0; i < 256; i++)
    {
        buff[i] = '\0';
    }
    buff_count = 0;
    
    comm = !comm;
    tb_hide_cursor();
    update_debug_console();
    return 0;
    
}




void initialize() {
    tb_init();
    tb_set_input_mode(TB_INPUT_ESC);
    tb_set_output_mode(TB_OUTPUT_GRAYSCALE);
    tb_clear();
}

void update_debug_console(){

    render_layout();
    actualize_memory();
    
    char buff[300];
    sprintf(buff, reg_w.fmt, regs.general[acc],regs.general[bcc],regs.general[r1],regs.general[r2],
            regs.general[r3],regs.general[r4],regs.general[hx],regs.general[lx]);
    render_widget(reg_w.x, reg_w.y, buff);

    sprintf(buff, pc_w.fmt, regs.pc);
    render_widget(pc_w.x, pc_w.y, buff);

    sprintf(buff, current_w.fmt, "NOT IMPLEMENTED");
    render_widget(current_w.x, current_w.y, buff);

    render_flags(regs.status);
    render_memory();
    render_stack();

    render_pc_cursor(regs.pc);

    if (comm) {

        get_command(buffer, buff_w);
    }

       
    tb_present();    
}

void debugger_step(){

    cycle();
    update_debug_console();
}
void debugger_step_back(){
    regs.pc = prev_pc;
}

int main() {

    st.pointer = 0;

    /*#define len_prog 6
    byte_t prog[len_prog] = {
        0x11, 0x18, 0x5, 0x1c, 0xf, 0x0
    };*/
    
    /*for (size_t i = 0; i < len_prog; i++)
    {
        memory[i] = prog[i];    
    }
*/
    

    for (uint16_t i = 0; i < 32; i++)
    {
        memory_w_row[i] = i;
        for (uint16_t j = 0; j < 16; j++)
        {
            memory_w[i][j] = memory[(i << 4) | j];
        }
        
    }
        
    regs.status = 0x2;

    initialize();

  

    update_debug_console();
    // DEBBUGER MAIN LOOP    
    while (1)
    {
        struct tb_event ev;
        tb_poll_event(&ev);

        switch (ev.key)
        {
        case TB_KEY_ESC:
            tb_shutdown();
            return 0;
            break;

        case TB_KEY_TAB:
            debugger_step();
            track_pc_cursor(regs.pc);
            update_debug_console();
            break;
        
        case TB_KEY_BACK_TAB:
            debugger_step_back();
            track_pc_cursor(regs.pc);
            update_debug_console();
            break;
        
        case TB_KEY_ARROW_DOWN:
            memory_w_scroll_down();
            update_debug_console();
            break;
        
        case TB_KEY_ARROW_UP:
            memory_w_scroll_up();
            update_debug_console();
            break;
        
        case TB_KEY_CTRL_P:
            track_pc_cursor(regs.pc);
            update_debug_console();
            break;
        case TB_KEY_CTRL_X:
            comm = !comm;
            tb_hide_cursor();
            update_debug_console();
            break;

        default:
            break;
        }

        /*int err = cycle();

        if (err) { return err;}*/
    }
}