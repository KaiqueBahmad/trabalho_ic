# Relatório de Implementação — O Peso da Coroa

## Visão Geral

O Peso da Coroa é um RPG textual em C onde o jogador interpreta um rei governando um reino em crise diplomática e militar. O projeto foi desenvolvido com foco em acessibilidade para usuários com deficiência visual.

## Decisões de Implementação

### Arquitetura Modular

O código foi dividido em módulos com responsabilidades claras: `jogo.c` coordena o loop principal; `locais.c` contém toda a lógica de exploração; `npcs.c` gerencia os diálogos; `salvamento.c` isola o sistema de persistência; `ui.c` centraliza toda saída de texto; `audio.c` abstrai o TTS. Isso facilitou o desenvolvimento incremental e a identificação de bugs.

### Acessibilidade como Prioridade

A decisão mais importante foi tratar acessibilidade como requisito central, não como adição posterior. Todos os textos passam pela mesma função `ui_narrar`/`ui_msg`, que pode futuramente acionar o TTS. Caracteres decorativos foram removidos do início ao fim para compatibilidade com leitores de tela como o NVDA.

### TTS com piper-tts

O piper foi escolhido por rodar localmente (sem internet), ter modelo em português brasileiro de qualidade adequada (`pt_BR-faber-medium`) e ser distribuível junto com o projeto. A integração usa arquivos temporários (`piper_in.txt`, `piper_out.wav`) e `system()` para invocar o executável, mantendo o código C simples e sem dependências de biblioteca nativa de TTS.

### Portabilidade Cross-platform

Uso de `#ifdef _WIN32` e `#ifdef __APPLE__` para isolar código específico de plataforma. O script `compile.sh` detecta o SO via `uname` e baixa o binário correto do piper. No Windows, `PlaySoundA` toca o WAV de forma síncrona; no macOS, `afplay`; no Linux, `aplay` com fallback para `paplay`.

### Formato de Salvamento

O formato `CHAVE=VALOR` em arquivo `.txt` foi escolhido por ser legível por humanos (facilita depuração), simples de implementar com `fprintf`/`sscanf`, e facilmente extensível. Todos os flags de eventos e a configuração de áudio são persistidos, garantindo que o estado seja completamente restaurável.

### Narrativa e Finais

Cinco finais foram implementados para garantir rejogabilidade e que escolhas tenham impacto real. O final secreto (5 - A Voz da Rainha) recompensa jogadores que exploram todas as possibilidades. A estrutura de flags booleanas no `Jogador` (como `traidor_revelado`, `prisioneiro_libertado`) torna as condições de final verificáveis de forma simples e eficiente.

O Final 4 (derrota) é acionado automaticamente ao atingir 25 turnos sem resolução, criando pressão real de tempo que reflete o ultimato de Drakmar na narrativa. Isso garante que o final de derrota seja alcançável independentemente das escolhas do jogador.
