# Guia do Jogador — O Peso da Coroa

Você é o novo soberano de Avalon, **um entre seis reinos** que disputam estas terras. Seu papel é governar: plantar, criar gado, encher o celeiro, equilibrar os impostos, abrir minas de ouro, erguer muralhas e forjar um exército. Os reinos rivais crescem, se espionam e guerreiam entre si — e podem marchar sobre você. **Só haverá paz quando uma única coroa reinar sobre todas as terras: a sua.** O objetivo é **anexar todos os reinos** e tornar-se imperador.

## Como Jogar

O jogo é totalmente em texto e pensado para ser jogado também só pelo áudio. A cada estação você ouve um relatório do reino e escolhe ações digitando o número correspondente (ou o nome) e pressionando Enter. Algumas ações pedem uma quantidade em seguida.

O tempo passa em estações: **Primavera** (o gado se reproduz e a terra é preparada), **Verão** (a lavoura amadurece), **Outono** (a colheita enche o celeiro) e **Inverno** (o povo come do que foi guardado). Os **impostos são recolhidos a cada estação** e, ao fim de cada uma, as **minas** apuram quanto ouro renderam. Quando estiver pronto, escolha **Avançar para a próxima estação** — é também quando os reinos rivais agem.

Com a narração por voz ligada, aperte a tecla **ESC** a qualquer momento para pular a fala atual e seguir em frente.

## O que você administra

- **População** — seus habitantes: mão de obra para a lavoura, base dos impostos e futuros soldados. Cresce com fartura; encolhe na fome ou com impostos altos.
- **Comida (celeiro)** — cada inverno o povo (e os soldados) consome grãos. Se faltar, há **fome** e gente morre. Planejar a reserva é o coração do jogo.
- **Gado** — o rebanho cresce sozinho a cada ano (limitado pelo pasto, que depende dos campos). Pode ser vendido por ouro ou abatido por comida: é sua poupança viva.
- **Campos** — produzem grãos na colheita. Mais campos rendem mais comida e sustentam mais gado, mas precisam de gente para serem lavrados.
- **Ouro** — vem dos impostos, do comércio e das **minas**. Paga obras, gado, minas, muralhas e o exército.
- **Imposto** — sua principal alavanca, recolhido **toda estação**. Imposto baixo atrai moradores e faz o povo crescer; imposto alto enche o cofre, mas faz gente migrar para outras terras.
- **Minas** — exigem alto investimento e **cada nova mina custa mais que a anterior**, mas rendem ouro ao fim de cada estação, somado direto ao tesouro.
- **Soldados e muralha** — sua força no mapa. A muralha tem **vida** e defende (mas não ataca); quanto maior o nível, mais vida. Geridos no **Quartel**.

## Comandos a qualquer momento

| Comando            | O que faz                                        |
|--------------------|--------------------------------------------------|
| `guia`             | Explica as regras do reino (estações, gado, impostos, reinos, quartel, minas) |
| `situacao`         | Relê o estado completo do reino                  |
| `opcoes`           | Repete as ações disponíveis na estação           |
| `repetir`          | Repete a última narração por voz                 |
| `salvar`           | Salva o reino em arquivo                         |
| `audio`            | Ativa ou desativa a narração por voz             |
| `velocidade`       | Alterna a velocidade da narração (normal / rápida / muito rápida); a escolha é lembrada entre sessões |
| `ajuda`            | Mostra os comandos                               |
| `sair`             | Encerra o jogo                                   |

## O Quartel

No **Quartel** ficam as ações militares: **recrutar soldados** (cada um sai da lavoura e come do celeiro no inverno), **dispensar soldados** de volta ao trabalho e **reforçar a muralha**. A muralha sobe de nível (mais vida) e, se for danificada por um ataque, pode ser reparada. A muralha defende, mas **não pode ser enviada para atacar**.

## Os Reinos do mapa

No menu **Reinos** você lida com as outras cinco civilizações:

- **Atacar** — você escolhe quantos soldados enviar. Se mandar **mais tropas do que a defesa do inimigo** (soldados + vida da muralha), você o **conquista e anexa**: perde tropas iguais à defesa dele e leva **um terço** da população, do ouro, da comida e do gado (até onde houver pasto). Se mandar **de menos**, **perde todas as tropas enviadas** — mas o inimigo também perde defensores (a muralha é corroída primeiro, depois os soldados).
- **Obter informações (espionar)** — paga ouro para descobrir **um dado aleatório** sobre o reino escolhido (tropas, muralha, tesouro, minas...).
- **Jornal** — as declarações de guerra e os resultados das batalhas do mundo, mais recentes primeiro.

Os reinos rivais também **constroem minas e muralhas, recrutam tropas e atacam** uns aos outros e você. No primeiro ano há uma trégua para você se firmar.

## Dicas

- Encha o celeiro **antes** do inverno: a colheita prevista aparece no relatório.
- Equilibre o imposto: por volta de 15 por cento o reino cresce; muito acima disso, o povo foge.
- Use o gado como reserva para anos ruins de clima.
- **Minas** são um investimento de longo prazo: quanto antes abrir, mais estações de ouro elas rendem.
- **Espione antes de atacar**: saber quantos soldados e que muralha o inimigo tem evita mandar tropas de menos e perdê-las.
- Mantenha **muralha e guarnição** sempre de pé — os rivais podem invadir a qualquer estação a partir do ano 2.
- Salve com frequência usando `salvar`.

## Finais

- **Imperador de Avalon** — você anexou todos os reinos e unificou as terras. Vitória.
- **Reino Conquistado** — um rival invadiu e tomou o seu reino.
- **Reino Despovoado** — você deixou o povo morrer de fome ou partir.
