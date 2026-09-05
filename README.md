
## Arquitetura Base do Sistema

- ### Ponto de Montagem Virtual (Aberto):
  #### Exemplo: ~/meu_drive_seguro
  Pasta virtual visível ao usuário onde os arquivos e diretórios aparecem decifrados em texto claro. O usuário interage com essa pasta normalmente (cria, edita, exclui e abre arquivos).

- ### Pasta de Persistência Física (Cifrada): 
  #### Exemplo: ~/GoogleDrive/pasta_cifrada
  Pasta física real no disco que serve de base para o FUSE. Todos os arquivos nesta pasta têm seus nomes e conteúdos completamente criptografados. Esta pasta é a que o cliente de sincronização (ex:   Google Drive) envia automaticamente para a nuvem.


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
  |                 Nosso Módulo Criptográfico                    |
  |          [ KDF (SHA-512) -> AES-CTR -> HMAC-SHA256 ]          |
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
  
  O FUSE é um módulo do Kernel do Linux que atua como uma ponte de redirecionamento de chamadas de sistema (syscalls). A sua função no Nosso Módulo Criptográfico é:
  
    - Interceptação Transparente: Quando um aplicativo (como o terminal ou uma IDE) tenta acessar um arquivo no ponto de montagem virtual (~/meu_drive_seguro), o Kernel intercepta essa chamada.
    - Encaminhamento de Chamadas: Em vez de tratar essa requisição diretamente nos drivers físicos de disco, o FUSE desvia a chamada para a nossa aplicação escrita em C que roda em espaço de         usuário.
    - Retorno do Dado Processado: Nosso programa processa o dado (faz a validação do HMAC e a decifragem com AES-CTR) e devolve o texto claro de volta ao Kernel através do FUSE, que por sua vez entrega   ao aplicativo solicitante de forma totalmente transparente.

    <br>
    <br>


    | Comando do Usuário | Execução no Linux | Função callback a ser implementada|
    | :--- | :--- | :--- |
    | `ls` (listar arquivos) | Solicita listagem do diretório e metadados de cada arquivo. | `readdir` (para decifrar os nomes dos arquivos) e `getattr` (para carregar permissões e tamanhos). |
    | `cd pasta` (entrar em pasta) | Verifica a existência e propriedades do caminho. | `getattr` (para responder ao Linux que o caminho físico cifrado é de fato um diretório). |
    | `cat arquivo.txt` (ler arquivo) | Abre o arquivo, lê uma sequência de bytes e fecha. | `open` (valida permissões), `read` (decifra em RAM com AES-CTR) e `release` (fecha o descritor de arquivo). |
    | `echo "dados" > arq.txt` | Cria um novo arquivo físico e grava dados. | `create` (cria arquivo físico na pasta oculta), `write` (cifra com AES-CTR e calcula HMAC) e `release`. |
    | `mkdir nova_pasta` | Solicita a criação de um diretório físico. | `mkdir` (cria diretório com nome cifrado na pasta de persistência real). |
    | `rm arquivo.txt` | Solicita a remoção de um arquivo físico. | `unlink` (localiza e remove o arquivo criptografado correspondente na pasta física). |



* ### Layout de Bytes do Arquivo Cifrado
  
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
    |                      TAG DE AUTENTICAÇÃO (32 bytes)                     |
    |                              (HMAC-SHA-256)                             |
    +-------------------------------------------------------------------------+
    ```
