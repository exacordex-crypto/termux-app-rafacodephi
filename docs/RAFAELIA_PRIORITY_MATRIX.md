# RAFAELIA · Matriz de prioridade vetorial para termux-app-rafacodephi

Objetivo: aplicar triagem por evidência, urgência, latência e obviedade no fluxo Android/Termux, especialmente instalação, SDK, bootstrap e início da sessão terminal.

## Vetor base

```text
v = (intencao, observacao, ruido, transmutacao, memoria, coerencia, urgencia, latencia, evidencia, obviedade)
```

## Fórmula de prioridade

```text
valor = 0.20*evidencia + 0.18*urgencia + 0.16*memoria + 0.14*transmutacao + 0.12*latencia + 0.08*obviedade + 0.06*intencao + 0.04*ruido + 0.02*coerencia
```

## Prioridades deste repositório

| Camada | Aplicação | Critério de prova |
|---|---|---|
| Urgência | app instala mas não inicia sessão de terminal | reproduzir crash/falha e registrar logcat |
| Evidência | build Android, APK, runtime e sessão shell | APK abre sessão com log mínimo limpo |
| Obviedade | SDK mínimo/target, permissões, ABI, bootstrap | README com comandos e ambiente |
| Latência | integração RAFCODEphi, sensores, developer mode, debugger runtime | mapear pontos reais no código |
| Ruído fértil | erro "Android SDK 29", permissões, path, bootstrap quebrado | transformar cada erro em hotfix rastreável |

## Blending por componente

| Componente | Tratamento |
|---|---|
| Gradle/SDK | conferir compileSdk, minSdk, targetSdk e plugins |
| Terminal session | localizar classe/serviço que cria PTY/shell |
| Bootstrap | validar path, permissões, env e arquivos iniciais |
| Runtime Android | capturar logcat e exceções |
| Sensores/API | separar permissão, chamada Java/Kotlin e camada nativa |
| RAFCODEphi | manter como camada de identidade/configuração, sem mascarar falha técnica |

## Estados canônicos

```text
[INSTALA]      APK instala
[ABRE]         app abre sem crash inicial
[SESSAO_OK]    terminal cria sessão funcional
[FALHA_SDK]    erro relacionado a SDK/Gradle/API level
[FALHA_PTY]    erro na criação de shell/PTTY
[FALHA_BOOT]   bootstrap/path/env quebrado
[HOTFIX]       correção mínima aplicável
[PROVADO]      correção testada com log
```

## Próximo ciclo

1. Localizar fluxo de criação de sessão terminal.
2. Identificar dependências SDK 29 e chamadas incompatíveis.
3. Criar checklist de build e runtime.
4. Registrar logcat mínimo para falhas de sessão.
5. Aplicar hotfixes pequenos, testáveis e reversíveis.

## Retroalimentar[3]

- F_ok: a falha de início de sessão agora tem matriz de ataque.
- F_gap: falta ligar cada item a arquivos reais do código.
- F_next: varrer classes de terminal/bootstrap e criar hotfix específico.
