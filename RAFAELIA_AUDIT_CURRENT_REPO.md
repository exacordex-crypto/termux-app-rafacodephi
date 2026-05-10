# RAFAELIA Audit — Current Repo

- **Data**: 2026-05-10
- **Repo atual**: `termux-app-rafacodephi`
- **Branch atual**: `work`
- **Commit atual (baseline desta rodada)**: `89bd736`

## Inventário
- Build: Gradle Android multi-módulo (`app`, `termux-shared`, `terminal-emulator`, `terminal-view`, `rafaelia`, `rmr`).
- Nativo: NDK/ndkBuild + bootstrap rewrite pipeline.
- Linguagens: Java/Kotlin, C/C++, ASM, Shell, Python.
- ABIs declaradas: `armeabi-v7a`, `arm64-v8a`, `x86_64` (+ `x86` opcional).

## Comandos executados e resultado real
1. `./gradlew :app:compileDebugJavaWithJavac`
   - **PASSOU** (após correções desta e da rodada anterior).
2. `./gradlew :app:assembleDebug`
   - **PASSOU**.
   - APK debug gerado com pipeline completo (rewrite/validate/compile/package).
3. `./gradlew :app:assembleRelease`
   - **FALHOU** em `:app:validateBootstrapBlake3Config` por ausência de `TERMUX_BOOTSTRAP_BLAKE3_*`.
4. `eval "$(./scripts/prepare_bootstrap_env.sh --print-env)" && ./gradlew :app:assembleRelease`
   - **FALHOU** novamente no mesmo gate.
   - Observação: script reportou inconsistência de bootstrap (`missing entries: ['SYMLINKS.txt']`) antes do build.
5. `./gradlew verifyReleaseContract`
   - **FALHOU**: `dist/apk-matrix` ausente; exige execução prévia de `scripts/build_apk_matrix.sh`.

## Causas-raiz fechadas nesta rodada
- Cadeia de compile debug foi estabilizada e concluída.
- Integração `BuildConfig`/Java/resources alinhada para fechar `compileDebugJavaWithJavac` e `assembleDebug`.

## Causas-raiz ainda abertas
1. Gate de release bloqueado por variáveis `TERMUX_BOOTSTRAP_BLAKE3_*` ausentes na prática.
2. Pipeline de preparo de bootstrap acusando `SYMLINKS.txt` ausente em bootstrap zip esperado.
3. `verifyReleaseContract` depende de matriz de artefatos (`dist/apk-matrix`) ainda não gerada nesta execução.

## Estado medido
- **Build debug**: passou.
- **Build release unsigned**: falhou no gate de hash bootstrap.
- **Build release signed**: não validado (sem credenciais de assinatura neste ambiente).
- **Arm32/arm64 em device real**: não validado neste ambiente.

## Relação com outros repos
- Próximo repo recomendado permanece `termux-packages` para contrato/layout de bootstrap e alinhamento estrito de metadados.
