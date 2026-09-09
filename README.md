
## Arquitetura Base do Sistema

- ### Ponto de Montagem Virtual (Aberto):
  #### Exemplo: ~/meu_drive_seguro
  Pasta virtual visível ao usuário onde os arquivos e diretórios aparecem decifrados em texto claro. O usuário interage com essa pasta normalmente (cria, edita, exclui e abre arquivos).

- ### Pasta de Persistência Física (Cifrada): 
  #### Exemplo: ~/GoogleDrive/pasta_cifrada
  Pasta física real no disco que serve de base. Todos os arquivos nesta pasta têm seus nomes e conteúdos completamente criptografados. Esta pasta é a que o cliente de sincronização (ex:   Google Drive) envia automaticamente para a nuvem.


- Diagrama:
  ```text
  +---------------------------------------------------------------+
  |                           Usuário                             |
  |                   (via IDE, terminal, etc)                    |
  +---------------------------------------------------------------+
                                  ▲
                                  | Acesso e Edição de Arquivos Comuns (Sem Cifragem)
                                  ▼
  +---------------------------------------------------------------+
  |                      FUSE (Kernel Module)                     |
  +---------------------------------------------------------------+
                                  ▲
                                  | (Interceptação e Redirecionamento de Chamadas(syscalls))
                                  ▼
  +---------------------------------------------------------------+
  |                     Módulo Criptográfico                      |
  |          [ KDF (SHA-512) -> AES-CTR -> HMAC-SHA512 ]          |
  +---------------------------------------------------------------+
                                  ▲
                                  | (Escrita/Leitura Cifrada - Ex: ~/GoogleDrive)
                                  ▼
  +---------------------------------------------------------------+
  |                  Pasta Local Sincronizada com                 |
  |                          Google Drive                         |
  +---------------------------------------------------------------+
  ```
  

## Estrutura da implementação.

* ### Ideia: usar o FUSE (Filesystem in Userspace)

  A implementação de um sistema de arquivos criptografado transparente poderia ser feita de duas formas: operando diretamente dentro do núcleo do sistema operacional (espaço de Kernel) ou operando no espaço de usuário (User Space). Pretendo seguir com a segunda abordagem por meio do FUSE (Filesystem in Userspace).
  
  O FUSE é um módulo do Kernel do Linux que atua como uma ponte de redirecionamento de chamadas de sistema (syscalls). A sua função no Módulo Criptográfico é:
  
    - Interceptação Transparente: Quando um aplicativo (como o terminal ou uma IDE) tenta acessar um arquivo no ponto de montagem virtual (~/meu_drive_seguro), o Kernel intercepta essa chamada.
    - Encaminhamento de Chamadas: Em vez de tratar essa requisição diretamente nos drivers físicos de disco, o FUSE desvia a chamada para a nossa aplicação escrita em C que roda em espaço de usuário.
    - Retorno do Dado Processado: Nosso programa processa o dado (faz a validação do HMAC e a decifragem com AES-CTR) e devolve o texto claro de volta ao Kernel através do FUSE, que por sua vez entrega ao aplicativo solicitante de forma totalmente transparente.

    <br>
    <br>


    | Comando do Usuário | Execução no Linux | Função callback a ser implementada|
    | :--- | :--- | :--- |
    | `ls` (listar arquivos) | Solicita listagem do diretório e metadados de cada arquivo. | `readdir` (para decifrar os nomes dos arquivos) e `getattr` (para carregar permissões e tamanhos). |
    | `cd pasta` (entrar em pasta) | Verifica a existência e propriedades do caminho. | `getattr` (para responder ao Linux que o caminho físico cifrado é de fato um diretório). |
    | `cat arquivo.txt` (ler arquivo) | Abre o arquivo, lê uma sequência de bytes e fecha. | `open` (valida permissões), `read` (decifra com AES-CTR) e `release` (fecha o descritor de arquivo). |
    | `echo "dados" > arq.txt` | Cria um novo arquivo físico e grava dados. | `create` (cria arquivo físico na pasta oculta), `write` (cifra com AES-CTR e calcula HMAC) e `release`. |
    | `mkdir nova_pasta` | Solicita a criação de um diretório físico. | `mkdir` (cria diretório com nome cifrado na pasta de persistência real). |
    | `rm arquivo.txt` | Solicita a remoção de um arquivo físico. | `unlink` (localiza e remove o arquivo criptografado correspondente na pasta física). |

* ### Alternativas ao FUSE (ainda em espaço de usuário):
  - Interceptação de Chamadas via LD_PRELOAD (Sobrescrita da libc):
      Construção de uma biblioteca compartilhada (.so em C) que sobrescreve as funções padrão de manipulação de arquivos da biblioteca libc (open, read, write, close). Ao executar qualquer programa com o comando LD_PRELOAD=./sua_lib.so aplicativo, a biblioteca intercepta as chamadas de I/O em tempo de execução, cifrando e decifrando os dados na memória de forma transparente apenas para aquele processo. **Desvantagem:** só funciona nos processos iniciados explicitamente sob esse wrapper e falha se o programa fizer chamadas de sistema (syscalls) diretas sem passar pela libc, o que pode expor problemas para programas compilados estaticamente ou que rodem chamadas diretas do sistema em assembly.

  - Servidor de Rede Loopback Local (WebDAV ou NFS em User-Space):
      Execução de um servidor WebDAV ou NFS local rodando em background. O próprio gerenciador de arquivos do sistema operacional conecta nessa pasta de rede virtual como se fosse um servidor remoto. **Desvantagem:** possível overhead dos protocolos de rede (HTTP/WebDAV) para operações simples de disco local.

  O uso do FUSE ainda me parece a melhor opção, uma vez que funciona apenas como um redirecionador padronizado pelo próprio Linux, redirencionando as syscalls diretamente para a nossa aplicação.

* ### Funcionamento da Criptografia em si

    O sistema divide a criptografia em três componentes básicos de implementação simplificada e de código próprio:

    1. Derivação e Separação de Chaves (KDF)
       
       - A partir de uma única senha digitada pelo usuário no momento da montagem, a partir de uma saída de 64 bytes, geram-se duas chaves criptográficas distintas e independentes de 32 bytes cada (uma para cifragem com o AES e a outra para o HMAC):
    3. Cifragem com AES-CTR (Counter Mode)
       
       - A cifragem de dados utiliza o algoritmo AES-256 como PRP. Foi escolhida a implementação standalone "tiny-AES-c" (https://github.com/kokke/tiny-AES-C)
    5. Autenticação de Integridade (HMAC)
       - Em uma callback write, o nosso código calcula: Tag = HMAC-SHA-512<sub>k<sub>hmac</sub></sub>(Salt || IV || Ciphertext). Esta Tag de 64 bytes é anexada pelo nosso programa ao final do arquivo físico no disco.
       - Em uma callback read, o nosso código reconstrói a Tag sobre os dados do arquivo e a compara com a Tag armazenada.
    <br>
  
    ```text
    +-------------------------------------------------------------------------+
    |                         SALT da KDF (32 bytes)                          |
    +-------------------------------------------------------------------------+
    |                 VETOR DE INICIALIZAÇÃO / IV (16 bytes)                  |
    +-------------------------------------------------------------------------+
    |                                                                         |
    |                      DADOS CRIPTOGRAFADOS (N bytes)                     |
    |                            (Payload AES-CTR)                            |
    |                                                                         |
    +-------------------------------------------------------------------------+
    |                      TAG DE AUTENTICAÇÃO (64 bytes)                     |
    |                              (HMAC-SHA-512)                             |
    +-------------------------------------------------------------------------+
    ```
