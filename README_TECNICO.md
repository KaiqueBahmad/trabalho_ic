# README Técnico — O Peso da Coroa

## Estrutura do Projeto

```
trabalho_ic/
├── main.c                  Ponto de entrada
├── compile.bat             Script de compilação e download do piper (Windows)
├── compile.sh              Script de compilação e download do piper (Linux/macOS)
├── audio/
│   ├── audio.h
│   └── audio.c             TTS via piper-tts (cross-platform)
├── src/
│   ├── jogo.h / jogo.c     Loop principal, menu, globais
│   ├── jogador.h / jogador.c   Estado do jogador
│   ├── inventario.h / inventario.c  Inventário de itens
│   ├── locais.h / locais.c     Locais do mapa e processamento de comandos
│   ├── npcs.h / npcs.c         Diálogos dos NPCs
│   ├── salvamento.h / salvamento.c  Salvar/carregar em saves/save.txt
│   ├── entrada.h / entrada.c   Leitura, normalização e confirmação de input
│   ├── ui.h / ui.c             Exibição de texto, opções e narração
│   └── utils.h / utils.c       Detecção de sistema operacional
├── models/
│   ├── pt_BR-faber-medium.onnx
│   └── pt_BR-faber-medium.onnx.json
└── saves/                  Criado automaticamente ao salvar
```

## Compilação

### Windows
```bat
compile.bat
```
Baixa o piper automaticamente se necessário e compila com MinGW/GCC.

### Linux / macOS
```sh
bash compile.sh
```
Detecta o SO via `uname`, baixa o piper correto e compila com GCC.

### Comando GCC manual (Windows)
```
gcc main.c audio/audio.c src/jogador.c src/inventario.c src/entrada.c src/ui.c src/salvamento.c src/npcs.c src/locais.c src/jogo.c src/utils.c -I. -o main.exe -lwinmm
```

### Comando GCC manual (Linux/macOS)
```
gcc main.c audio/audio.c src/jogador.c src/inventario.c src/entrada.c src/ui.c src/salvamento.c src/npcs.c src/locais.c src/jogo.c src/utils.c -I. -o main
```

## Dependências

- GCC (MinGW no Windows)
- piper-tts (baixado automaticamente pelos scripts)
- Modelo de voz: `models/pt_BR-faber-medium.onnx` (incluído no repositório)
- Windows: `-lwinmm` para `PlaySoundA`

## Decisões de Implementação

### Acessibilidade
- Todo texto relevante é narrado via TTS (piper-tts com voz pt-BR)
- Opções são anunciadas no formato "Opção N, texto." para criar pausas naturais na leitura
- Nenhum caractere decorativo (`==`, `--`, `>>`, `[...]`, `(...)`) no output — compatível com leitores de tela
- Beep de 880 Hz ao aguardar input (apenas Windows, apenas com áudio ativado)
- Comando `repetir` reproduz a última narração

### Áudio
- Pergunta se o usuário deseja áudio ao iniciar
- Se piper ou o modelo não forem encontrados, o jogo continua sem áudio
- Configuração de áudio é salva e carregada com o jogo
- Comando `audio` alterna a narração a qualquer momento

### Salvamento
- Formato human-readable `CHAVE=VALOR` em `saves/save.txt`
- Salva todos os atributos, flags de eventos, inventário e configuração de áudio
- Comando `salvar` disponível a qualquer momento durante o jogo
- Comando `carregar` disponível a qualquer momento durante o jogo

### Cross-platform
- `#ifdef _WIN32 / __APPLE__` para código específico de plataforma
- `PlaySoundA` no Windows, `afplay` no macOS, `aplay`/`paplay` no Linux
- `_mkdir` no Windows, `mkdir` no Linux/macOS
- Scripts de compilação detectam SO automaticamente

## Atributos do Jogador

| Atributo       | Inicial | Descrição                        |
|----------------|---------|----------------------------------|
| Vida           | 100     | Saúde do rei                     |
| Ouro           | 200     | Recursos financeiros             |
| Exército       | 50      | Força militar                    |
| Popularidade   | 60      | Apoio popular                    |
| Sabedoria      | 30      | Conhecimento acumulado           |

## Condições de Derrota

- Vida ≤ 0
- Ouro ≤ 0 E Exército ≤ 10 simultaneamente

## Finais Disponíveis

1. Vitória Militar — exército forte, batalha vencida
2. Paz Diplomática — traidor preso, Erik como intermediário, negociação com Drakmar
3. Herói do Povo — alta popularidade, povo se mobiliza
4. Queda de Avalon — 25 turnos esgotados sem resolução, ou vida ≤ 0, ou ouro ≤ 0 com exército ≤ 10
5. A Voz da Rainha (secreto) — carta de Erik chega à rainha de Drakmar
