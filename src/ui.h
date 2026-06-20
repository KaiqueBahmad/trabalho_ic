#ifndef UI_H
#define UI_H

void ui_separador(void);
void ui_titulo(const char *titulo);
void ui_narrar(const char *texto);
void ui_msg(const char *texto);
void ui_erro(const char *texto);
void ui_opcao(int num, const char *texto);
void ui_repetir(void);
void ui_ajuda(void);
void ui_opcoes_globais(void);
void ui_falar_opcoes(void);
void ui_limpar_opcoes(void);
void ui_prompt_menu(const char *pergunta);

#endif
