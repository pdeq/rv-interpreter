#ifndef HEADERS
#define HEADERS
#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "label_table.h"
#include "hash_table.h"
#endif

#include "instructions.h"
#include "format.h"

int sim_PC = 0;
char **code_lines;
int total_lines;

typedef struct
{
  GtkWidget *scroll;
  GtkWidget *text_view;
  GtkWidget *output;
  GtkWidget *memory_output;
  GtkWidget *memory_input;
} AppWidgets;

void update_memory_display(AppWidgets *app_widgets, uint64_t start_address)
{
  GtkTextBuffer *memory_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_widgets->memory_output));
  GString *memory_text = g_string_new(NULL);

  for (int i = 0; i <= 30; ++i)
  {
    uint64_t address = start_address + i;
    uint64_t value = ht_get(memory, address);
    g_string_append_printf(memory_text, "Memory[0x%" PRIx64 "] = 0x%02" PRIx64 "\n", address, value & 0xFF);
  }

  gtk_text_buffer_set_text(memory_buffer, memory_text->str, -1);
  g_string_free(memory_text, TRUE);
}

void on_memory_input_activate(GtkEntry *entry, gpointer data)
{
  AppWidgets *app_widgets = (AppWidgets *)data;
  const char *text = gtk_entry_get_text(entry);
  uint64_t start_address;

  if (sscanf(text, "%" SCNx64, &start_address) == 1)
  {
    update_memory_display(app_widgets, start_address);
  }
}

void update_registers_display(AppWidgets *app_widgets)
{
  GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_widgets->output));
  GString *registers_text = g_string_new(NULL);

  for (int i = 0; i < 32; ++i)
  {
    if (i < 10)
    {
      g_string_append_printf(registers_text, "x%d:  0x%" PRIx64 "\n", i, registers[i]);
    }
    else
    {
      g_string_append_printf(registers_text, "x%d: 0x%" PRIx64 "\n", i, registers[i]);
    }
  }

  gtk_text_buffer_set_text(output_buffer, registers_text->str, -1);
  g_string_free(registers_text, TRUE);
}

int step(char *instruction, int line)
{
  while (isspace(*instruction))
  {
    instruction++;
  }
  char *op = strsep(&instruction, " ");
  int op_type = get_op_type(op);

  if (op_type == R_TYPE)
  {
    if (!validate_format(instruction, 3, 0))
    {
      printf("Malformed instruction.\nPlease reset.\n");
      return INT_MIN;
    }
    char *tokens = strtok(instruction, ", ");
    char *rd_s = tokens;
    int rd = get_register(rd_s);
    tokens = strtok(NULL, ", ");
    char *rs1_s = tokens;
    int rs1 = get_register(rs1_s);
    tokens = strtok(NULL, ", ");
    char *rs2_s = tokens;
    int rs2 = get_register(rs2_s);

    if (rd == -1 || rs1 == -1 || rs2 == -1)
    {
      return INT_MIN;
    }

    if (rd == 0)
    {
      return line + 4;
    }

    if (strcmp(op, "add") == 0)
    {
      r_add(rd, rs1, rs2);
    }
    else if (strcmp(op, "sub") == 0)
    {
      r_sub(rd, rs1, rs2);
    }
    else if (strcmp(op, "and") == 0)
    {
      r_and(rd, rs1, rs2);
    }
    else if (strcmp(op, "or") == 0)
    {
      r_or(rd, rs1, rs2);
    }
    else if (strcmp(op, "xor") == 0)
    {
      r_xor(rd, rs1, rs2);
    }
    else if (strcmp(op, "slt") == 0)
    {
      r_slt(rd, rs1, rs2);
    }
    else if (strcmp(op, "sltu") == 0)
    {
      r_sltu(rd, rs1, rs2);
    }
    else if (strcmp(op, "sll") == 0)
    {
      r_sll(rd, rs1, rs2);
    }
    else if (strcmp(op, "sra") == 0)
    {
      r_sra(rd, rs1, rs2);
    }
    else if (strcmp(op, "srl") == 0)
    {
      r_srl(rd, rs1, rs2);
    }
    else if (strcmp(op, "addw") == 0)
    {
      r_addw(rd, rs1, rs2);
    }
    else if (strcmp(op, "sllw") == 0)
    {
      r_sllw(rd, rs1, rs2);
    }
    else if (strcmp(op, "srlw") == 0)
    {
      r_srlw(rd, rs1, rs2);
    }
    else if (strcmp(op, "subw") == 0)
    {
      r_subw(rd, rs1, rs2);
    }
    else if (strcmp(op, "sraw") == 0)
    {
      r_sraw(rd, rs1, rs2);
    }
  }
  else if (op_type == I_TYPE)
  {
    if (!validate_format(instruction, 3, 0))
    {
      printf("Malformed instruction.\nPlease reset.\n");
      return INT_MIN;
    }
    char *tokens = strtok(instruction, ", ");
    char *rd_s = tokens;
    int rd = get_register(rd_s);
    tokens = strtok(NULL, ", ");
    char *rs1_s = tokens;
    int rs1 = get_register(rs1_s);
    tokens = strtok(NULL, ", ");
    if (rd == -1 || rs1 == -1)
    {
      return INT_MIN;
    }
    char *imm_s = tokens;
    uint64_t imm;
    if (strncmp(imm_s, "0x", 2) == 0)
    {
      if (!(is_hex_digits(imm_s + 2)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 16);
    }
    else if (strncmp(imm_s, "-0x", 3) == 0)
    {
      if (!(is_hex_digits(imm_s + 3)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 16);
    }
    else
    {
      if (!(is_decimal_digits(imm_s)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 10);
    }
    if ((int64_t)imm > 2047 || (int64_t)imm < -2048)
    {
      printf("Immediate is too large.\nExpected in range [-2048, 2047].\nPlease reset.\n");
      return INT_MIN;
    }

    if (rd == 0)
    {
      return line + 4;
    }

    if (strcmp(op, "addi") == 0)
    {
      r_addi(rd, rs1, imm);
    }
    else if (strcmp(op, "andi") == 0)
    {
      r_andi(rd, rs1, imm);
    }
    else if (strcmp(op, "ori") == 0)
    {
      r_ori(rd, rs1, imm);
    }
    else if (strcmp(op, "xori") == 0)
    {
      r_xori(rd, rs1, imm);
    }
    else if (strcmp(op, "slti") == 0)
    {
      r_slti(rd, rs1, imm);
    }
    else if (strcmp(op, "sltiu") == 0)
    {
      r_sltiu(rd, rs1, imm);
    }
    else if (strcmp(op, "addiw") == 0)
    {
      r_addiw(rd, rs1, imm);
    }
    else if (strcmp(op, "slli") == 0)
    {
      r_slli(rd, rs1, imm);
    }
    else if (strcmp(op, "slliw") == 0)
    {
      r_slliw(rd, rs1, imm);
    }
    else if (strcmp(op, "srli") == 0)
    {
      r_srli(rd, rs1, imm);
    }
    else if (strcmp(op, "srliw") == 0)
    {
      r_srliw(rd, rs1, imm);
    }
    else if (strcmp(op, "srai") == 0)
    {
      r_srai(rd, rs1, imm);
    }
    else if (strcmp(op, "sraiw") == 0)
    {
      r_sraiw(rd, rs1, imm);
    }
  }
  else if (op_type == MEM_TYPE)
  {
    if (!validate_parentheses_format(instruction))
    {
      printf("Malformed instruction.\nPlease reset.\n");
      return INT_MIN;
    }
    // rd or rs2
    char *tokens = strtok(instruction, ", ()");
    char *rds2_s = tokens;
    int rds2 = get_register(rds2_s);
    tokens = strtok(NULL, ", ()");
    char *imm_s = tokens;
    uint64_t imm;
    if (strncmp(imm_s, "0x", 2) == 0)
    {
      if (!(is_hex_digits(imm_s + 2)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 16);
    }
    else if (strncmp(imm_s, "-0x", 3) == 0)
    {
      if (!(is_hex_digits(imm_s + 3)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 16);
    }
    else
    {
      if (!(is_decimal_digits(imm_s)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 10);
    }
    if ((int64_t)imm > 2047 || (int64_t)imm < -2048)
    {
      printf("Immediate is too large.\nExpected in range [-2048, 2047].\nPlease reset.\n");
      return INT_MIN;
    }
    tokens = strtok(NULL, ", ()");
    char *rs1_s = tokens;
    int rs1 = get_register(rs1_s);

    if (rds2 == -1 || rs1 == -1)
    {
      return INT_MIN;
    }

    if (registers[rs1] + imm > 0x7FFFFFFFFFFFFFFF)
    {
      printf("Valid memory addresses are between 0x0 and 0x7FFFFFFFFFFFFFFF\nPlease reset.\n");
      return INT_MIN;
    }

    if (strcmp(op, "lb") == 0)
    {
      if (rds2 == 0)
      {
        return line + 4;
      }
      r_lb(rds2, rs1, imm);
    }
    else if (strcmp(op, "lh") == 0)
    {
      if (rds2 == 0)
      {
        return line + 4;
      }
      r_lh(rds2, rs1, imm);
    }
    else if (strcmp(op, "lw") == 0)
    {
      if (rds2 == 0)
      {
        return line + 4;
      }
      r_lw(rds2, rs1, imm);
    }
    else if (strcmp(op, "ld") == 0)
    {
      if (rds2 == 0)
      {
        return line + 4;
      }
      r_ld(rds2, rs1, imm);
    }
    else if (strcmp(op, "sb") == 0)
    {
      r_sb(rs1, rds2, imm, 0);
    }
    else if (strcmp(op, "sh") == 0)
    {
      r_sh(rs1, rds2, imm);
    }
    else if (strcmp(op, "sw") == 0)
    {
      r_sw(rs1, rds2, imm);
    }
    else if (strcmp(op, "sd") == 0)
    {
      r_sd(rs1, rds2, imm);
    }
  }
  else if (op_type == U_TYPE)
  {
    if (!validate_format(instruction, 2, 0))
    {
      printf("Malformed instruction.\nPlease reset.\n");
      return INT_MIN;
    }
    char *tokens = strtok(instruction, ", ");
    char *rd_s = tokens;
    int rd = get_register(rd_s);
    if (rd == -1)
    {
      return INT_MIN;
    }
    tokens = strtok(NULL, ", ");
    char *imm_s = tokens;
    uint64_t imm;
    if (strncmp(imm_s, "0x", 2) == 0)
    {
      if (!(is_hex_digits(imm_s + 2)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 16);
    }
    else
    {
      if (!(is_decimal_digits(imm_s)))
      {
        printf("Not a valid immediate.\nPlease reset.\n");
        return INT_MIN;
      }
      imm = strtoull(imm_s, NULL, 10);
    }

    if (imm > 0xFFFFF)
    {
      printf("Immediate is too large.\nExpected in range [0, 1048575].\nPlease reset.\n");
      return INT_MIN;
    }

    if (rd == 0)
    {
      return line + 4;
    }

    if (strcmp(op, "lui") == 0)
    {
      r_lui(rd, imm);
    }
    else if (strcmp(op, "auipc") == 0)
    {
      r_auipc(rd, imm, line);
    }
  }
  else if (op_type == B_TYPE)
  {
    if (!validate_format(instruction, 3, 1))
    {
      printf("Malformed instruction.\nPlease reset.\n");
      return INT_MIN;
    }
    char *tokens = strtok(instruction, ", ");
    char *rs1_s = tokens;
    int rs1 = get_register(rs1_s);
    tokens = strtok(NULL, ", ");
    char *rs2_s = tokens;
    int rs2 = get_register(rs2_s);
    if (rs1 == -1 || rs2 == -1)
    {
      return INT_MIN;
    }
    tokens = strtok(NULL, ", ");
    char *imm_s = tokens;
    uint64_t imm = imm_label(imm_s);
    imm = read_offset(tokens, imm_s, imm);

    if (imm % 4 || (int64_t)imm < 0)
    {
      printf("Must branch to instruction that exists.\nPlease reset.\n");
      return INT_MIN;
    }

    if (strcmp(op, "beq") == 0)
    {
      return r_beq(rs1, rs2, imm, line);
    }
    else if (strcmp(op, "bne") == 0)
    {
      return r_bne(rs1, rs2, imm, line);
    }
    else if (strcmp(op, "blt") == 0)
    {
      return r_blt(rs1, rs2, imm, line);
    }
    else if (strcmp(op, "bltu") == 0)
    {
      return r_bltu(rs1, rs2, imm, line);
    }
    else if (strcmp(op, "bge") == 0)
    {
      return r_bge(rs1, rs2, imm, line);
    }
    else if (strcmp(op, "bgeu") == 0)
    {
      return r_bgeu(rs1, rs2, imm, line);
    }
    else if (strcmp(op, "bgt") == 0)
    {
      return r_blt(rs2, rs1, imm, line);
    }
    else if (strcmp(op, "bgtu") == 0)
    {
      return r_bltu(rs2, rs1, imm, line);
    }
    else if (strcmp(op, "ble") == 0)
    {
      return r_bge(rs2, rs1, imm, line);
    }
    else if (strcmp(op, "bleu") == 0)
    {
      return r_bgeu(rs2, rs1, imm, line);
    }
  }
  else if (op_type == BZ_TYPE)
  {
    if (!validate_format(instruction, 2, 1))
    {
      printf("Malformed instruction.\nPlease reset.\n");
      return INT_MIN;
    }
    char *tokens = strtok(instruction, ", ");
    char *rs_s = tokens;
    int rs = get_register(rs_s);
    if (rs == -1)
    {
      return INT_MIN;
    }
    tokens = strtok(NULL, ", ");
    char *imm_s = tokens;
    uint64_t imm = imm_label(imm_s);
    imm = read_offset(tokens, imm_s, imm);

    if (imm % 4 || (int64_t)imm < 0)
    {
      printf("Must branch to instruction that exists.\nPlease reset.\n");
      return INT_MIN;
    }

    if (strcmp(op, "beqz") == 0)
    {
      return r_beq(rs, 0, imm, line);
    }
    else if (strcmp(op, "bnez") == 0)
    {
      return r_bne(rs, 0, imm, line);
    }
    else if (strcmp(op, "blez") == 0)
    {
      return r_bge(0, rs, imm, line);
    }
    else if (strcmp(op, "bgez") == 0)
    {
      return r_bge(rs, 0, imm, line);
    }
    else if (strcmp(op, "bltz") == 0)
    {
      return r_blt(rs, 0, imm, line);
    }
    else if (strcmp(op, "bgtz") == 0)
    {
      return r_blt(0, rs, imm, line);
    }
  }
  else if (op_type == DR_TYPE)
  {
    if (!validate_format(instruction, 2, 0))
    {
      printf("Malformed instruction.\nPlease reset.\n");
      return INT_MIN;
    }
    char *tokens = strtok(instruction, ", ");
    char *rd_s = tokens;
    int rd = get_register(rd_s);
    tokens = strtok(NULL, ", ");
    char *rs_s = tokens;
    int rs = get_register(rs_s);
    if (rd == -1 || rs == -1)
    {
      return INT_MIN;
    }
    if (rd == 0)
    {
      return line + 4;
    }

    if (strcmp(op, "mv") == 0)
    {
      r_addi(rd, rs, 0);
    }
    else if (strcmp(op, "not") == 0)
    {
      r_xori(rd, rs, -1);
    }
    else if (strcmp(op, "neg") == 0)
    {
      r_sub(rd, 0, rs);
    }
    else if (strcmp(op, "negw") == 0)
    {
      r_subw(rd, 0, rs);
    }
    else if (strcmp(op, "sext.w") == 0)
    {
      r_addw(rd, rs, 0);
    }
    else if (strcmp(op, "seqz") == 0)
    {
      r_sltiu(rd, rs, 1);
    }
    else if (strcmp(op, "snez") == 0)
    {
      r_sltu(rd, 0, rs);
    }
    else if (strcmp(op, "sltz") == 0)
    {
      r_slt(rd, rs, 0);
    }
    else if (strcmp(op, "sgtz") == 0)
    {
      r_slt(rd, 0, rs);
    }
  }
  else if (op_type == J_TYPE)
  {
    if (strcmp(op, "ret") == 0)
    {
      if (instruction != NULL)
      {
        while (*instruction && isspace(*instruction))
        {
          instruction++;
        }
        if (instruction[0] != '#')
        {
          printf("Malformed instruction.\nPlease reset.\n");
          return INT_MIN;
        }
      }
      return r_jalr(0, 1, 0, line);
    }
    int is_one = validate_format(instruction, 1, 1);
    int is_one_no = validate_format(instruction, 1, 0);
    int is_two = validate_format(instruction, 2, 1);
    int is_three = validate_format(instruction, 3, 0);
    char *tokens = strtok(instruction, ", ");
    char *first = tokens;

    if (strcmp(op, "jal") == 0)
    {
      tokens = strtok(NULL, ", ");
      char *second = tokens;
      if (second == NULL || second[0] == '+' || second[0] == '-')
      {
        if (!is_one)
        {
          printf("Malformed instruction.\nPlease reset.\n");
          return INT_MIN;
        }
        uint64_t imm = imm_label(first);
        imm = read_offset(tokens, first, imm);
        if (imm % 4 || (int64_t)imm < 0)
        {
          printf("Must branch to instruction that exists.\nPlease reset.\n");
          return INT_MIN;
        }
        return r_jal(1, imm, line);
      }
      else
      {
        if (!is_two)
        {
          printf("Malformed instruction.\nPlease reset.\n");
          return INT_MIN;
        }
        uint64_t imm = imm_label(second);
        imm = read_offset(tokens, second, imm);
        if (imm % 4 || (int64_t)imm < 0)
        {
          printf("Must branch to instruction that exists.\nPlease reset.\n");
          return INT_MIN;
        }
        int rd = lt_get(abi_map, first);
        rd = rd == -1 ? atoi(first + 1) : rd;
        return r_jal(rd, imm, line);
      }
    }
    else if (strcmp(op, "jalr") == 0)
    {
      tokens = strtok(NULL, ", ");
      char *second = tokens;
      if (second == NULL || second[0] == '#')
      {
        if (!is_one_no)
        {
          printf("Malformed instruction.\nPlease reset.\n");
          return INT_MIN;
        }
        int rs = lt_get(abi_map, first);
        rs = rs == -1 ? atoi(first + 1) : rs;
        return r_jalr(1, rs, 0, line);
      }
      else
      {
        if (!is_three)
        {
          printf("Malformed instruction.\nPlease reset.\n");
          return INT_MIN;
        }
        int rd = lt_get(abi_map, first);
        rd = rd == -1 ? atoi(first + 1) : rd;
        int rs = lt_get(abi_map, second);
        rs = rs == -1 ? atoi(second + 1) : rs;
        tokens = strtok(NULL, ", ");
        char *imm_s = tokens;
        uint64_t imm = imm_label(imm_s);
        if (imm % 4 || (int64_t)imm < 0)
        {
          printf("Must branch to instruction that exists.\nPlease reset.\n");
          return INT_MIN;
        }
        return r_jalr(rd, rs, imm, line);
      }
    }
    else if (strcmp(op, "jr") == 0)
    {
      if (!is_one_no)
      {
        printf("Malformed instruction.\nPlease reset.\n");
        return INT_MIN;
      }
      int rs = lt_get(abi_map, first);
      rs = rs == -1 ? atoi(first + 1) : rs;
      return r_jalr(0, rs, 0, line);
    }
    else
    {
      uint64_t imm = imm_label(first);
      imm = read_offset(tokens, first, imm);
      if (strcmp(op, "j") == 0)
      {
        if (!is_one)
        {
          printf("Malformed instruction.\nPlease reset.\n");
          return INT_MIN;
        }
        if (imm % 4 || (int64_t)imm < 0)
        {
          printf("Must branch to instruction that exists.\nPlease reset.\n");
          return INT_MIN;
        }
        return r_jal(0, imm, line);
      }
    }
  }
  else
  {
    printf("Illegal instruction.\nPlease reset.\n");
    return INT_MIN;
  }
  return line + 4;
}

void step_core(AppWidgets *app_widgets)
{
  if (sim_PC < user_instruction_count * 4 && sim_PC >= 0)
  {
    int index = sim_PC / 4;
    char buffer[256];
    strcpy(buffer, user_instructions[index].instruction);
    printf("Processing line %d: %s\n", user_instructions[index].source_line + 1, buffer);
    for (int i = 0; buffer[i]; ++i)
    {
      buffer[i] = tolower(buffer[i]);
    }
    sim_PC = step(buffer, sim_PC);
  }
  else
  {
    printf("No more lines to process\n");
  }

  update_registers_display(app_widgets);
  update_memory_display(app_widgets, 0);
}

void on_step_button_clicked(GtkWidget *widget, gpointer data)
{
  AppWidgets *app_widgets = (AppWidgets *)data;
  GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_widgets->text_view));
  GtkTextIter start, end;
  GtkTextIter line_start, line_end;
  char *line_text;

  if (gtk_text_view_get_editable(GTK_TEXT_VIEW(app_widgets->text_view)))
  {
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    total_lines = gtk_text_buffer_get_line_count(input_buffer);

    code_lines = malloc(total_lines * sizeof(char *));
    for (int i = 0; i < total_lines; ++i)
    {
      gtk_text_buffer_get_iter_at_line(input_buffer, &line_start, i);
      if (i == total_lines - 1)
      {
        line_end = end;
      }
      else
      {
        gtk_text_buffer_get_iter_at_line(input_buffer, &line_end, i + 1);
      }
      line_text = gtk_text_buffer_get_text(input_buffer, &line_start, &line_end, FALSE);
      code_lines[i] = g_strdup(g_strchomp(line_text));
      g_free(line_text);
    }

    pass(code_lines, total_lines);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_widgets->text_view), FALSE);

    step_core(app_widgets);
  }
  else
  {
    step_core(app_widgets);
  }
}

void run_core(AppWidgets *app_widgets)
{
  while (sim_PC < user_instruction_count * 4 && sim_PC >= 0)
  {
    int index = sim_PC / 4;
    char buffer[256];
    strcpy(buffer, user_instructions[index].instruction);
    printf("Processing line %d: %s\n", user_instructions[index].source_line + 1, buffer);
    for (int i = 0; buffer[i]; ++i)
    {
      buffer[i] = tolower(buffer[i]);
    }
    sim_PC = step(buffer, sim_PC);
  }

  printf("No more lines to process.\n");

  update_registers_display(app_widgets);
  update_memory_display(app_widgets, 0);
}

void on_run_button_clicked(GtkWidget *widget, gpointer data)
{
  AppWidgets *app_widgets = (AppWidgets *)data;
  GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_widgets->text_view));
  GtkTextIter start, end;
  GtkTextIter line_start, line_end;
  char *line_text;

  if (gtk_text_view_get_editable(GTK_TEXT_VIEW(app_widgets->text_view)))
  {
    gtk_text_buffer_get_bounds(input_buffer, &start, &end);
    total_lines = gtk_text_buffer_get_line_count(input_buffer);

    code_lines = malloc(total_lines * sizeof(char *));
    for (int i = 0; i < total_lines; ++i)
    {
      gtk_text_buffer_get_iter_at_line(input_buffer, &line_start, i);
      if (i == total_lines - 1)
      {
        line_end = end;
      }
      else
      {
        gtk_text_buffer_get_iter_at_line(input_buffer, &line_end, i + 1);
      }
      line_text = gtk_text_buffer_get_text(input_buffer, &line_start, &line_end, FALSE);
      code_lines[i] = g_strdup(g_strchomp(line_text));
      g_free(line_text);
    }

    pass(code_lines, total_lines);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_widgets->text_view), FALSE);

    run_core(app_widgets);
  }
  else
  {
    run_core(app_widgets);
  }
}

void on_reset_button_clicked(GtkWidget *widget, gpointer data)
{
  printf("\n\n\n");
  AppWidgets *app_widgets = (AppWidgets *)data;
  gtk_text_view_set_editable(GTK_TEXT_VIEW(app_widgets->text_view), TRUE);

  for (int i = 0; i < 32; ++i)
  {
    registers[i] = 0;
  }

  ht_free(memory);
  memory = ht_init();

  lt_free(labels);
  labels = lt_init();

  if (user_instructions != NULL)
  {
    for (int i = 0; i < user_instruction_count; ++i)
    {
      free(user_instructions[i].instruction);
    }
    free(user_instructions);
    user_instructions = NULL;
  }
  user_instruction_count = 0;

  update_memory_display(app_widgets, 0);

  update_registers_display(app_widgets);

  if (code_lines != NULL)
  {

    for (int i = 0; i < total_lines; ++i)
    {
      g_free(code_lines[i]);
    }
    free(code_lines);
    code_lines = NULL;
  }

  total_lines = 0;

  sim_PC = 0;
}

int main(int argc, char *argv[])
{
  memory = ht_init();
  labels = lt_init();
  abi_map = lt_init();
  char abis[32][5] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
  char defs[32][4] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"};

  for (int i = 0; i < 32; ++i)
  {
    lt_insert(abi_map, defs[i], i);
    lt_insert(abi_map, abis[i], i);
  }
  lt_insert(abi_map, "fp", 8);

  if (argc > 1)
  {
    FILE *user_asm = fopen(argv[1], "r");
    if (!user_asm)
    {
      printf("The provided file, %s, is invalid.\n", argv[1]);
      return 1;
    }

    total_lines = 0;
    char line[256];
    while (fgets(line, sizeof(line), user_asm))
    {
      total_lines++;
    }

    code_lines = malloc(total_lines * sizeof(char *));

    rewind(user_asm);

    int line_index = 0;
    while (fgets(line, sizeof(line), user_asm))
    {
      int len = strlen(line);
      if (len > 0 && line[len - 1] == '\n')
      {
        line[len - 1] = '\0';
      }

      code_lines[line_index] = malloc(len + 1);
      strcpy(code_lines[line_index], line);
      line_index++;
    }

    fclose(user_asm);

    pass(code_lines, total_lines);

    while (sim_PC < user_instruction_count * 4 && sim_PC >= 0)
    {
      int index = sim_PC / 4;
      char buffer[256];
      strcpy(buffer, user_instructions[index].instruction);
      printf("Processing line %d: %s\n", user_instructions[index].source_line + 1, buffer);
      for (int i = 0; buffer[i]; ++i)
      {
        buffer[i] = tolower(buffer[i]);
      }
      sim_PC = step(buffer, sim_PC);
    }

    if (sim_PC != INT_MIN)
    {
      printf("Register values:\n");
      for (int i = 0; i < 32; ++i)
      {
        printf("R[%d] = 0x%" PRIx64 "\n", i, registers[i]);
      }
    }

    for (int i = 0; i < total_lines; ++i)
    {
      free(code_lines[i]);
    }
    free(code_lines);
    ht_free(memory);
    lt_free(abi_map);
    lt_free(labels);

    return 0;
  }

  gtk_init(&argc, &argv);

  GtkWidget *window;
  GtkWidget *paned;
  GtkWidget *right_paned;
  GtkWidget *left_vbox;
  GtkWidget *right_vbox;
  GtkWidget *memory_vbox;
  GtkWidget *run_button;
  GtkWidget *step_button;
  GtkWidget *reset_button;
  GtkWidget *button_box;
  GtkWidget *left_label;
  GtkWidget *right_label;
  GtkWidget *memory_label;

  AppWidgets *app_widgets = g_new(AppWidgets, 1);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "RISC-V Interpreter");
  gtk_window_set_default_size(GTK_WINDOW(window), 1200, 600);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  right_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(window), paned);

  left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  memory_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  gtk_paned_add1(GTK_PANED(paned), left_vbox);
  gtk_paned_add2(GTK_PANED(paned), right_paned);
  gtk_paned_add1(GTK_PANED(right_paned), right_vbox);
  gtk_paned_add2(GTK_PANED(right_paned), memory_vbox);

  left_label = gtk_label_new("Input your assembly code here:");
  gtk_widget_set_halign(left_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(left_vbox), left_label, FALSE, FALSE, 5);

  app_widgets->scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(app_widgets->scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  app_widgets->text_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_widgets->text_view), GTK_WRAP_WORD_CHAR);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(app_widgets->text_view), TRUE);
  gtk_container_add(GTK_CONTAINER(app_widgets->scroll), app_widgets->text_view);
  gtk_widget_set_vexpand(app_widgets->scroll, TRUE);
  gtk_box_pack_start(GTK_BOX(left_vbox), app_widgets->scroll, TRUE, TRUE, 0);

  button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(left_vbox), button_box, FALSE, FALSE, 0);

  run_button = gtk_button_new_with_label("Run");
  g_signal_connect(run_button, "clicked", G_CALLBACK(on_run_button_clicked), app_widgets);
  gtk_box_pack_start(GTK_BOX(button_box), run_button, TRUE, TRUE, 0);

  step_button = gtk_button_new_with_label("Step");
  g_signal_connect(step_button, "clicked", G_CALLBACK(on_step_button_clicked), app_widgets);
  gtk_box_pack_start(GTK_BOX(button_box), step_button, TRUE, TRUE, 0);

  reset_button = gtk_button_new_with_label("Reset");
  g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), app_widgets);
  gtk_box_pack_start(GTK_BOX(button_box), reset_button, TRUE, TRUE, 0);

  right_label = gtk_label_new("Registers:");
  gtk_widget_set_halign(right_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(right_vbox), right_label, FALSE, FALSE, 5);

  app_widgets->output = gtk_text_view_new();
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(app_widgets->output), TRUE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(app_widgets->output), FALSE);
  gtk_widget_set_vexpand(app_widgets->output, TRUE);
  gtk_box_pack_start(GTK_BOX(right_vbox), app_widgets->output, TRUE, TRUE, 0);

  memory_label = gtk_label_new("Memory:");
  gtk_widget_set_halign(memory_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(memory_vbox), memory_label, FALSE, FALSE, 5);

  app_widgets->memory_input = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(app_widgets->memory_input), "Enter memory address (hex)");
  g_signal_connect(app_widgets->memory_input, "activate", G_CALLBACK(on_memory_input_activate), app_widgets);
  gtk_box_pack_start(GTK_BOX(memory_vbox), app_widgets->memory_input, FALSE, FALSE, 0);

  app_widgets->memory_output = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(app_widgets->memory_output), FALSE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(app_widgets->memory_output), TRUE);
  gtk_widget_set_vexpand(app_widgets->memory_output, TRUE);
  gtk_box_pack_start(GTK_BOX(memory_vbox), app_widgets->memory_output, TRUE, TRUE, 0);

  gtk_paned_set_position(GTK_PANED(paned), 400);
  gtk_paned_set_position(GTK_PANED(right_paned), 300);

  gtk_widget_show_all(window);
  update_memory_display(app_widgets, 0);
  update_registers_display(app_widgets);
  gtk_main();

  g_free(app_widgets);
  return 0;
}
