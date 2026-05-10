# Engenharia de Estado Unificado (ARMÉ)

Este documento unifica os conceitos de `estado_nucleo.md`, `estado_aprendizado_fronteira.md`, `estado_modelo_temporal.md` e `conceitos_mvp.txt` em um contrato único:

- Linguagem como transição de estados.
- Ruído como sinal de fronteira para ajuste estrutural.
- Tempo como efeito de reorganização do sistema.
- Coerência multi-camada: semântica, lógica, estrutural e operacional.

## Contratos por arquivo (separação por responsabilidade)

### estado_nucleo.md — Núcleo Filosófico-Operacional
- Define interpretação por estados e transições.
- Define coerência local e global.

### estado_aprendizado_fronteira.md — Modelo de Aprendizado por Fronteira
- Ruído, inconsistência e ajuste incremental.
- Métricas de estabilidade sob variação.

### estado_modelo_temporal.md — Modelo Temporal e Multi-Representação
- Tempo como reorganização recorrente.
- Integração de texto, número, símbolo e imagem.

### conceitos_mvp.txt — Critérios de Solidez
- Robustez sob transformação de contexto.
- Persistência de forma com variação de escala.

## Regra de refatoração
Cada arquivo mantém foco próprio, mas incorpora as lacunas dos demais via seção **Interoperabilidade**.
