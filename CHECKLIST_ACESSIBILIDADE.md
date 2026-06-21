# Checklist de Acessibilidade — O Peso da Coroa

Este documento verifica a conformidade do jogo com os requisitos de acessibilidade, especialmente para usuários com deficiência visual.

---

## Navegação por Teclado

- [x] Todo o jogo é controlado exclusivamente pelo teclado
- [x] Nenhuma interação requer mouse
- [x] Comandos aceitos tanto por número quanto por texto (ex: `1` ou `novo`)
- [x] Entradas com maiúsculas, acentos ou espaços extras são normalizadas automaticamente

---

## Compatibilidade com Leitores de Tela

- [x] Saída de texto sem caracteres decorativos (`==`, `--`, `>>`, `[...]`, `(...)`, `|`)
- [x] Sem arte ASCII que confunda a leitura linear
- [x] Sem caixas de texto ou formatação de tabela no output do jogo
- [x] Opções apresentadas no formato simples: `N - texto`
- [x] Mensagens de erro em texto plano: `Erro: texto`
- [x] Títulos de seções em texto plano sem delimitadores
- [x] Confirmações apresentadas com opções numeradas claras

---

## Narração por Voz (TTS)

- [x] Pergunta ao usuário se deseja ativar áudio ao iniciar
- [x] Jogo funciona normalmente sem áudio (modo texto puro)
- [x] Se o piper não estiver instalado, o jogo inicia sem áudio e informa o usuário
- [x] Toda narrativa importante é narrada via TTS quando áudio está ativo
- [x] Opções narradas no formato "Opção N, texto." (ponto final cria pausa natural)
- [x] Comando `repetir` reproduz a última narração
- [x] Comando `audio` alterna a narração a qualquer momento durante o jogo
- [x] Configuração de áudio é salva junto com o jogo
- [x] Sinal sonoro (880 Hz) ao aguardar input — apenas quando áudio ativo (Windows)

---

## Feedback ao Usuário

- [x] Todas as ações do jogador geram resposta textual visível
- [x] Entradas inválidas exibem mensagem de erro clara
- [x] Confirmações pedem resposta explícita antes de ações destrutivas (ex: sair, carregar)
- [x] Status do jogo acessível a qualquer momento pelo comando `status`
- [x] Inventário acessível a qualquer momento pelo comando `inventario`
- [x] Opções do local atual repetíveis com `opcoes`

---

## Comandos de Acessibilidade

| Comando    | Disponível | Descrição                              |
|------------|------------|----------------------------------------|
| `ajuda`    | Sempre     | Lista todos os comandos                |
| `status`   | Sempre     | Exibe atributos do personagem          |
| `inventario` | Sempre   | Lista itens                            |
| `examinar` | Sempre     | Descreve itens do inventário           |
| `repetir`  | Sempre     | Repete a última narração por voz       |
| `opcoes`   | Sempre     | Repete as opções do local atual        |
| `salvar`   | Sempre     | Salva o progresso                      |
| `carregar` | Sempre     | Carrega jogo salvo                     |
| `audio`    | Sempre     | Ativa/desativa narração                |
| `sair`     | Sempre     | Encerra o jogo com confirmação         |

---

## Idioma e Clareza

- [x] Todo o jogo em português brasileiro
- [x] Narração em português brasileiro (modelo `pt_BR-faber-medium`)
- [x] Textos de narrativa claros e sem ambiguidade
- [x] Nomes dos comandos em português
- [x] Dica sobre o comando `ajuda` exibida ao iniciar novo jogo
