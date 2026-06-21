# Documento de Testes Manuais — O Peso da Coroa

## Como Executar

Windows: `compile.bat` depois `main.exe`
Linux/macOS: `bash compile.sh` depois `./main`

---

## TM-01 — Menu Principal

**Objetivo:** Verificar que o menu inicial funciona corretamente.

| Passo | Ação | Resultado Esperado |
|-------|------|--------------------|
| 1 | Iniciar o jogo | Pergunta sobre narração por voz (se piper disponível) |
| 2 | Digitar `2` (sem áudio) | Confirma desativação e exibe menu principal |
| 3 | Digitar `1` | Pergunta o nome do rei, inicia novo jogo |
| 4 | Reiniciar e digitar `2` | Tenta carregar save; mensagem de erro se não existir |
| 5 | Reiniciar e digitar `3` | Exibe lista de ajuda |
| 6 | Reiniciar e digitar `0` | Encerra o programa |
| 7 | Digitar opção inválida (ex: `9`) | Mensagem de erro, menu repetido |

---

## TM-02 — Comandos Globais

**Objetivo:** Verificar que todos os comandos globais funcionam em qualquer local.

| Comando | Resultado Esperado |
|---------|--------------------|
| `ajuda` | Lista completa de comandos |
| `status` | Exibe vida, ouro, exército, popularidade, sabedoria, turno |
| `inventario` | Lista itens ou "inventário vazio" |
| `examinar` | Descreve cada item ou "inventário vazio" |
| `repetir` | Reproduz última narração |
| `opcoes` | Lista opções do local atual |
| `salvar` | Cria/atualiza `saves/save.txt` com mensagem de confirmação |
| `carregar` | Pede confirmação; carrega save e exibe local salvo |
| `audio` | Alterna narração e exibe estado atual |
| `sair` | Pede confirmação; encerra se confirmado |

---

## TM-03 — Locais e Navegação

**Objetivo:** Verificar acesso a todos os locais.

| Passo | Ação | Resultado Esperado |
|-------|------|--------------------|
| 1 | No Salão do Trono, digitar `2` | Vai para Câmara do Conselho |
| 2 | Digitar `1` (voltar) | Retorna ao Salão do Trono |
| 3 | Digitar `3` | Vai para o Quartel |
| 4 | Digitar `1` (voltar) | Retorna ao Salão do Trono |
| 5 | Digitar `4` | Vai para a Masmorra |
| 6 | Digitar `1` (voltar) | Retorna ao Salão do Trono |
| 7 | Digitar `5` | Vai para as Muralhas |
| 8 | Digitar `1` (voltar) | Retorna ao Salão do Trono |
| 9 | Digitar `6` | Vai para a Vila |
| 10 | Digitar `1` (voltar) | Retorna ao Salão do Trono |
| 11 | Digitar `7` | Vai para a Biblioteca |
| 12 | Digitar `1` (voltar) | Retorna ao Salão do Trono |

---

## TM-04 — Inventário e Itens

**Objetivo:** Verificar coleta e exame de itens.

| Passo | Ação | Resultado Esperado |
|-------|------|--------------------|
| 1 | Iniciar novo jogo, digitar `inventario` | "Inventário vazio" |
| 2 | Ir ao Quartel, digitar `2` (pagar soldados) | Reduz ouro; pode adicionar item |
| 3 | Digitar `inventario` | Lista item obtido |
| 4 | Digitar `examinar` | Descrição detalhada do item |

---

## TM-05 — Salvamento e Carregamento

**Objetivo:** Verificar que o estado é preservado entre sessões.

| Passo | Ação | Resultado Esperado |
|-------|------|--------------------|
| 1 | Iniciar jogo, executar algumas ações | Atributos modificados |
| 2 | Digitar `salvar` | Arquivo `saves/save.txt` criado |
| 3 | Digitar `sair` e confirmar | Jogo encerra |
| 4 | Iniciar jogo novamente, digitar `2` (carregar) | Estado anterior restaurado |
| 5 | Verificar `status` | Atributos iguais ao momento do save |
| 6 | Verificar local | Local igual ao momento do save |
| 7 | Durante o jogo, digitar `carregar` | Pede confirmação; restaura save |

---

## TM-06 — NPCs

**Objetivo:** Verificar interações com todos os personagens.

| NPC | Local | Verificação |
|-----|-------|-------------|
| General Marcus | Quartel | Diálogo disponível, oferece opções de treinamento |
| Tobias (conselheiro) | Câmara do Conselho | Diálogo disponível; comportamento suspeito mencionado |
| Erik (prisioneiro) | Masmorra | Diálogo disponível; libertação requer item correto |
| Emissário de Drakmar | Salão do Trono (turno 1) | Aparece automaticamente com ultimato |

---

## TM-07 — Finais

**Objetivo:** Verificar que todos os finais são alcançáveis.

| Final | Condição Principal | Resultado Esperado |
|-------|--------------------|--------------------|
| 1 - Vitória Militar | Exército alto, batalha no Quartel | Texto de vitória militar |
| 2 - Paz Diplomática | Traidor revelado + Erik libertado | Texto de tratado de paz |
| 3 - Herói do Povo | Popularidade alta, povo ajudado | Texto de mobilização popular |
| 4 - Queda de Avalon | Vida ≤ 0 ou (ouro ≤ 0 e exército ≤ 10) | Texto de derrota |
| 5 - A Voz da Rainha | Carta de Erik enviada à rainha | Texto do final secreto |

---

## TM-08 — Acessibilidade

**Objetivo:** Verificar compatibilidade com leitores de tela.

| Verificação | Método | Resultado Esperado |
|-------------|--------|--------------------|
| Sem caracteres decorativos | Ler saída do jogo | Nenhum `==`, `--`, `>>`, `[...]`, `(...)`, `\|` |
| Opções numeradas | Iniciar local qualquer | Formato "N - texto" |
| Comando `repetir` funciona | Digitar `repetir` após narração | Última frase narrada repetida |
| Narração de opções | Com áudio ativo | "Opção N, texto." com pausa entre opções |
| Entrada inválida | Digitar `xyzzy` | Mensagem de erro clara, sem travar |

---

## TM-09 — Entrada Inválida

**Objetivo:** Verificar robustez contra entradas inesperadas.

| Entrada | Resultado Esperado |
|---------|--------------------|
| String vazia (só Enter) | Nenhum erro; aguarda próximo comando |
| Número fora das opções (ex: `99`) | Mensagem de opção inválida |
| Texto sem sentido (`asdfgh`) | Mensagem de comando não reconhecido |
| Comando com maiúsculas (`AJUDA`) | Funciona igual ao minúsculo |
| Comando com acento (`ação`) | Normalizado e processado |
| Espaços extras (`  salvar  `) | Normalizado e processado |
