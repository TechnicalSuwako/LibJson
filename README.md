# LibJSON

C++20で作ったミニマリストなJSONライブラリ

## インストールする方法
### Windowsの場合
```powershell
> cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
> cmake --build build --config Release
> mkdir -Force (自分のプロジェクトパス)\deps\libjson\lib
> mkdir -Force (自分のプロジェクトパス)\deps\libjson\include
> ROBOCOPY .\build\Release\libjson.lib (自分のプロジェクトパス)\deps\libjson\lib
> ROBOCOPY .\include (自分のプロジェクトパス)\deps\libjson /E /COPYALL
```

### Unixの場合
```sh
$ cmake -B build -DCMAKE_BUILD_TYPE=Release
$ cmake --build build --config Release
$ cmake --install build --config Release
```

## 確認出来た環境
|          | x64 | AArch64 | RISC-V | POWER ISA | Loongarch | SPARC64 |
|----------|-----|---------|--------|-----------|-----------|---------|
| Windows  | ☑   | ❓      | -      | -         | -         | -       |
| macOS    | -   | ☑       | -      | -         | -         | -       |
| Linux    | ☑   | ❓      | ❓     | ❓        | ❓        | ❓      |
| FreeBSD  | ☑   | ❓      | ❓     | ❓        | -         | -       |
| OpenBSD  | ☑   | ❓      | ❓     | ❓        | ❓        | ❓      |
| NetBSD   | ❓  | ❓      | -      | ❓        | ❓        | ❓      |
| Switch   | -   | ☑       | -      | -         | -         | -       |
| Switch 2 | -   | ☑       | -      | -         | -         | -       |
