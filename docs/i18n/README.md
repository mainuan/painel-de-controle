# Internacionalização (i18n)

* **API:** KF6 KI18n. `appwindow.ui` é compilado com `uic -tr tr2i18n`
  (`QMAKE_UIC_FLAGS` em `ki18n.pri`), e `appwindow.cpp` inclui
  `<KLocalizedString>` antes de `ui_appwindow.h`. `main.cpp` chama
  `KLocalizedString::setApplicationDomain("painel-controle")` antes do
  `QApplication`.
* **Domínio gettext:** `painel-controle` (nome do executável instalado,
  `Exec=painel-controle`).
* **Idioma-fonte:** inglês. A interface brasileira está em `po/pt_BR.po`, com os
  rótulos portugueses originais.
* Os rótulos dos botões também são o `--caption` passado ao `kcmshell6`, então
  o título da janela do KCM segue o idioma. As listas `kcms` (`kcm_*`,
  `guvcview`) são identificadores e estão marcadas `notr="true"` no `.ui`.

## Layout

```
ki18n.pri            KI18n para qmake (inclui/linka KF6 KI18n, uic -tr tr2i18n)
po/i18n.sh           script padrão Mainuan (extract/update/validate/…)
po/i18n.conf         DOMAIN=painel-controle
po/POTFILES.in       appwindow.ui
po/LINGUAS           idiomas, um por linha, sem comentários (hoje: pt_BR)
po/painel-controle.pot
po/pt_BR.po
tests/               teste de i18n (tst_i18n.pro + run-i18n-tests.sh)
```

## Build e instalação

KF6 não instala módulo qmake (`QT += KI18n`) nem arquivo pkg-config para o
KI18n; `ki18n.pri` usa `/usr/include/KF6/KI18n` e `-lKF6I18n` (mesmo caminho no
Kubuntu, Arch e Fedora). Outro prefixo: `qmake KF6_INCLUDEDIR=/prefixo/include/KF6`.
Dependências de build: `qt6-base-dev`, `libkf6i18n-dev`, `gettext`.

```sh
mkdir build && cd build
qmake6 ../PainelControle.pro && make
make install INSTALL_ROOT=/caminho/do/staging   # empacotamento
```

O `make` compila `po/<lang>.po` em `build/locale/<lang>/LC_MESSAGES/painel-controle.mo`;
o `make install` instala o executável (inalterado: `/opt/PainelControle/bin`) e
os catálogos em `$PREFIX/share/locale/<lang>/LC_MESSAGES/painel-controle.mo`
(`PREFIX=/usr` por padrão; `qmake PREFIX=…` para mudar). **Quem gera o
`PainelControle.deb` publicado nas releases precisa incluir esses `.mo`** (não
há `debian/` neste repositório).

Para rodar a partir do build sem instalar:

```sh
PAINEL_CONTROLE_LOCALEDIR=$PWD/locale LANGUAGE=pt_BR ./PainelControle
```

## Tarefas comuns

```sh
po/i18n.sh extract      # depois de mudar textos no .ui
po/i18n.sh update       # mescla o POT nos catálogos
po/i18n.sh validate     # msgfmt --check + placeholders
po/i18n.sh check-pot    # falha se o POT estiver desatualizado
po/i18n.sh add es       # novo idioma (Plural-Forms verificado) + LINGUAS
tests/run-i18n-tests.sh # compila e testa pt_BR, en e C (QT_QPA_PLATFORM=offscreen)
```

No Qt Designer, marque como "não traduzível" (`notr="true"`) folhas de estilo,
listas de KCMs e qualquer identificador. Para palavras curtas ambíguas use o
campo *disambiguation* (`comment=` no `.ui`, vira `msgctxt`).

## Particularidades

* `po/i18n.sh` é a cópia padrão com o extrator de `.ui` corrigido: ignora itens
  de `<stringlist notr="true">` e converte `comment=` em `msgctxt` (como o
  `tr2i18n` usa em tempo de execução).
* O teste pt_BR precisa de um locale instalado (`LANG=en_US.UTF-8`): o gettext
  ignora `LANGUAGE` quando o locale efetivo é `C`; sem ele o teste é pulado.
* O lançador `painel-controle.desktop` fica no repositório `dot-files`; o
  `Name` dele precisa ser traduzido lá (fonte em inglês `Control Panel`,
  `Name[pt_BR]=Painel de Controle`).
