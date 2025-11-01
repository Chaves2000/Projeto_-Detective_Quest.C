#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 Detective Quest - Mansão com pistas, hash de suspeitos e verificação final
 Autor: Enigma Studios (exemplo didático)
*/

// =======================
// Definições e limites
// =======================
#define MAX_NOME 64
#define MAX_PISTA 128
#define HASH_SIZE 101   // tamanho simples para a tabela hash (primo)

// =======================
// Estrutura da árvore de salas (mapa da mansão)
// =======================
typedef struct Sala {
    char nome[MAX_NOME];
    char pista[MAX_PISTA];    // pista associada à sala (pode ser string vazia)
    struct Sala* esquerda;
    struct Sala* direita;
} Sala;

// =======================
// Estrutura da BST de pistas coletadas
// =======================
typedef struct PistaNode {
    char pista[MAX_PISTA];
    struct PistaNode* esquerda;
    struct PistaNode* direita;
} PistaNode;

// =======================
// Estruturas para tabela hash (encadeamento)
// chave: pista (string), valor: suspeito (string)
// =======================
typedef struct HashNode {
    char pista[MAX_PISTA];
    char suspeito[MAX_NOME];
    struct HashNode* proximo;
} HashNode;

typedef struct {
    HashNode* buckets[HASH_SIZE];
} HashTable;

// =======================
// Funções para salas (mapa)
// =======================

/*
 criarSala()
 Cria dinamicamente um cômodo (Sala) com nome e pista (pode ser NULL ou "")
 Retorna ponteiro para a sala criada.
*/
Sala* criarSala(const char* nome, const char* pista) {
    Sala* s = (Sala*)malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro: falha na alocação de memória para Sala.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(s->nome, nome, MAX_NOME - 1);
    s->nome[MAX_NOME - 1] = '\0';
    if (pista && strlen(pista) > 0) {
        strncpy(s->pista, pista, MAX_PISTA - 1);
        s->pista[MAX_PISTA - 1] = '\0';
    } else {
        s->pista[0] = '\0';
    }
    s->esquerda = s->direita = NULL;
    return s;
}

// =======================
// Funções para BST de pistas
// =======================

/*
 inserirPista()
 Insere uma nova pista na BST de forma ordenada (alfabética).
 Não insere duplicatas exatas (compara strcmp).
 Retorna a raiz atualizada.
*/
PistaNode* inserirPista(PistaNode* raiz, const char* pista) {
    if (pista == NULL || strlen(pista) == 0) return raiz; // nada a inserir

    if (raiz == NULL) {
        PistaNode* n = (PistaNode*)malloc(sizeof(PistaNode));
        if (!n) {
            fprintf(stderr, "Erro: falha ao alocar nodo de pista.\n");
            exit(EXIT_FAILURE);
        }
        strncpy(n->pista, pista, MAX_PISTA - 1);
        n->pista[MAX_PISTA - 1] = '\0';
        n->esquerda = n->direita = NULL;
        return n;
    }

    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (cmp > 0) raiz->direita = inserirPista(raiz->direita, pista);
    // se igual: já existe, não insere duplicata
    return raiz;
}

/*
 exibirPistas()
 Imprime as pistas coletadas em ordem alfabética (in-order traversal).
*/
void exibirPistas(PistaNode* raiz) {
    if (raiz == NULL) return;
    exibirPistas(raiz->esquerda);
    printf(" - %s\n", raiz->pista);
    exibirPistas(raiz->direita);
}

/*
 liberarPistas()
 Libera memória da BST de pistas.
*/
void liberarPistas(PistaNode* raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

// =======================
// Funções para tabela hash
// =======================

/*
 hash_djb2()
 Função de hash (djb2) para strings - retorna índice entre 0 e HASH_SIZE-1.
*/
unsigned int hash_djb2(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + (unsigned char)c; /* hash * 33 + c */
    return (unsigned int)(hash % HASH_SIZE);
}

/*
 inicializarHash()
 Cria e inicializa uma tabela hash com buckets NULL.
*/
void inicializarHash(HashTable* ht) {
    for (int i = 0; i < HASH_SIZE; ++i) ht->buckets[i] = NULL;
}

/*
 inserirNaHash()
 Insere a associação pista -> suspeito na tabela hash.
 Se a pista já existir, atualiza o suspeito (sobrescreve).
*/
void inserirNaHash(HashTable* ht, const char* pista, const char* suspeito) {
    if (!pista || strlen(pista) == 0) return;
    unsigned int idx = hash_djb2(pista);
    HashNode* cur = ht->buckets[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) {
            // atualiza suspeito
            strncpy(cur->suspeito, suspeito, MAX_NOME - 1);
            cur->suspeito[MAX_NOME - 1] = '\0';
            return;
        }
        cur = cur->proximo;
    }
    // não encontrou: cria novo nó e insere no começo do bucket
    HashNode* novo = (HashNode*)malloc(sizeof(HashNode));
    if (!novo) {
        fprintf(stderr, "Erro: falha ao alocar HashNode.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(novo->pista, pista, MAX_PISTA - 1);
    novo->pista[MAX_PISTA - 1] = '\0';
    strncpy(novo->suspeito, suspeito, MAX_NOME - 1);
    novo->suspeito[MAX_NOME - 1] = '\0';
    novo->proximo = ht->buckets[idx];
    ht->buckets[idx] = novo;
}

/*
 encontrarSuspeito()
 Procura na hash o suspeito associado à pista.
 Retorna ponteiro para string do suspeito se encontrado, ou NULL.
*/
const char* encontrarSuspeito(HashTable* ht, const char* pista) {
    if (!pista || strlen(pista) == 0) return NULL;
    unsigned int idx = hash_djb2(pista);
    HashNode* cur = ht->buckets[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) return cur->suspeito;
        cur = cur->proximo;
    }
    return NULL;
}

/*
 liberarHash()
 Libera todos os nós da tabela hash.
*/
void liberarHash(HashTable* ht) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashNode* cur = ht->buckets[i];
        while (cur) {
            HashNode* tmp = cur;
            cur = cur->proximo;
            free(tmp);
        }
        ht->buckets[i] = NULL;
    }
}

// =======================
// Exploração e julgamento
// =======================

/*
 explorarSalas()
 Navega pela árvore de salas a partir de 'atual', coletando pistas automaticamente.
 Cada pista encontrada é adicionada à BST de pistas (via ponteiro pistasRaiz).
 Exibe informações ao jogador e possibilita escolher esquerda (e), direita (d) ou sair (s).
*/
void explorarSalas(Sala* atual, PistaNode** pistasRaiz) {
    if (!atual) {
        printf("Mapa vazio!\n");
        return;
    }

    char escolha;
    printf("\n=== Detective Quest: Exploração Final ===\n");
    printf("Iniciando no Hall de Entrada.\n\n");

    while (1) {
        printf("Você está em: %s\n", atual->nome);

        if (strlen(atual->pista) > 0) {
            printf("  -> Você encontrou uma pista: \"%s\"\n", atual->pista);
            // adiciona à BST de pistas coletadas
            *pistasRaiz = inserirPista(*pistasRaiz, atual->pista);
        } else {
            printf("  -> Nenhuma pista neste cômodo.\n");
        }

        // mostrar opções
        printf("\nOpções:\n");
        if (atual->esquerda) printf("  [e] Ir para %s (à esquerda)\n", atual->esquerda->nome);
        if (atual->direita)  printf("  [d] Ir para %s (à direita)\n", atual->direita->nome);
        printf("  [s] Sair da exploração (ir para julgamento)\n");
        printf("Escolha: ");
        if (scanf(" %c", &escolha) != 1) {
            // limpar stdin em caso de erro
            int c; while ((c = getchar()) != '\n' && c != EOF);
            escolha = 'x';
        }

        if (escolha == 'e' || escolha == 'E') {
            if (atual->esquerda) atual = atual->esquerda;
            else printf("  Não há caminho à esquerda.\n");
        } else if (escolha == 'd' || escolha == 'D') {
            if (atual->direita) atual = atual->direita;
            else printf("  Não há caminho à direita.\n");
        } else if (escolha == 's' || escolha == 'S') {
            printf("\nVocê encerrou a exploração e seguirá para o julgamento.\n");
            break;
        } else {
            printf("Opção inválida, tente novamente.\n");
        }

        printf("\n----------------------------------------\n");
    }
}

/*
 contarPistasParaSuspeito()
 Conta quantas pistas da BST correspondem ao suspeito fornecido, consultando a hash.
 Retorna contagem inteira.
*/
int contarPistasParaSuspeito(PistaNode* raiz, HashTable* ht, const char* suspeitoAlvo) {
    if (!raiz) return 0;
    int contador = 0;
    // percorrência in-order (pode ser qualquer ordem, estamos contando)
    contador += contarPistasParaSuspeito(raiz->esquerda, ht, suspeitoAlvo);
    const char* suspeito = encontrarSuspeito(ht, raiz->pista);
    if (suspeito != NULL && strcmp(suspeito, suspeitoAlvo) == 0) contador++;
    contador += contarPistasParaSuspeito(raiz->direita, ht, suspeitoAlvo);
    return contador;
}

/*
 verificarSuspeitoFinal()
 Recebe a BST de pistas, a hash de associações e o nome do suspeito acusado.
 Verifica se existem pelo menos 2 pistas que apontam para esse suspeito.
 Exibe mensagem de resultado.
*/
void verificarSuspeitoFinal(PistaNode* pistasRaiz, HashTable* ht, const char* acusado) {
    if (!acusado || strlen(acusado) == 0) {
        printf("Nome de suspeito inválido.\n");
        return;
    }

    int qtd = contarPistasParaSuspeito(pistasRaiz, ht, acusado);
    printf("\n=== Julgamento Final: %s ===\n", acusado);
    printf("Pistas que apontam para %s: %d\n", acusado, qtd);

    if (qtd >= 2) {
        printf("\nVeredicto: HÁ EVIDÊNCIAS SUFICIENTES para acusar %s!\n", acusado);
        printf("Parabéns, detetive — você reuniu indícios suficientes.\n");
    } else {
        printf("\nVeredicto: INSUFICIENTE. Não há pistas suficientes para condenar %s.\n", acusado);
        printf("Recomendação: continue investigando ou reavalie as pistas.\n");
    }
}

// =======================
// Funções utilitárias de liberação de salas
// =======================
void liberarSalas(Sala* raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    free(raiz);
}

// =======================
// MAIN - montagem do jogo
// =======================
int main() {
    // Montagem fixa do mapa (árvore binária)
    // Estrutura exemplo (pode ser adaptada):
    //
    //                 Hall
    //                /    \
    //           SalaEstar  Biblioteca
    //           /    \           \
    //       Cozinha  Jardim     Laboratório
    //         \
    //         Porão
    //

    Sala* hall        = criarSala("Hall de Entrada", "Pegadas misteriosas no tapete");
    Sala* salaEstar   = criarSala("Sala de Estar", "Um colar quebrado");
    Sala* biblioteca  = criarSala("Biblioteca", "Livro com páginas arrancadas");
    Sala* cozinha     = criarSala("Cozinha", "Faca com marcas de sangue");
    Sala* jardim      = criarSala("Jardim", "Luva de couro encontrada");
    Sala* porao       = criarSala("Porão Secreto", "Mapa antigo da mansão");
    Sala* laboratorio = criarSala("Laboratório", "Frascos com rótulos estranhos");

    // ligações
    hall->esquerda = salaEstar;
    hall->direita  = biblioteca;

    salaEstar->esquerda = cozinha;
    salaEstar->direita  = jardim;

    cozinha->direita = porao;
    biblioteca->direita = laboratorio;

    // inicializa BST de pistas coletadas (vazia)
    PistaNode* pistasRaiz = NULL;

    // inicializa e popula a tabela hash (pista -> suspeito)
    HashTable ht;
    inicializarHash(&ht);

    // associações definidas no código (simplificação)
    // OBS: use o texto exato das pistas como chave
    inserirNaHash(&ht, "Pegadas misteriosas no tapete", "Sr. Branco");
    inserirNaHash(&ht, "Um colar quebrado", "Sra. Rosa");
    inserirNaHash(&ht, "Livro com páginas arrancadas", "Prof. Verde");
    inserirNaHash(&ht, "Faca com marcas de sangue", "Sr. Branco");
    inserirNaHash(&ht, "Luva de couro encontrada", "Sr. Cinza");
    inserirNaHash(&ht, "Mapa antigo da mansão", "Sra. Rosa");
    inserirNaHash(&ht, "Frascos com rótulos estranhos", "Prof. Verde");

    // Iniciar exploração interativa
    explorarSalas(hall, &pistasRaiz);

    // Exibir pistas coletadas
    printf("\n=== Pistas coletadas (em ordem alfabética) ===\n");
    if (pistasRaiz == NULL) {
        printf("Nenhuma pista coletada.\n");
    } else {
        exibirPistas(pistasRaiz);
    }

    // Pergunta final: quem é o culpado?
    char acusado[MAX_NOME];
    printf("\nQuem você acusa como culpado? (digite o nome exato, ex: 'Sr. Branco'): ");
    // limpar buffer e ler linha (aceita espaços)
    int c; while ((c = getchar()) != '\n' && c != EOF); // limpar resto de entrada
    if (fgets(acusado, MAX_NOME, stdin) != NULL) {
        // remover '\n'
        size_t L = strlen(acusado);
        if (L > 0 && acusado[L-1] == '\n') acusado[L-1] = '\0';
    } else {
        acusado[0] = '\0';
    }

    // Verificar acusação (pelo menos 2 pistas apontando para o acusado)
    verificarSuspeitoFinal(pistasRaiz, &ht, acusado);

    // liberar memória
    liberarSalas(hall);
    liberarPistas(pistasRaiz);
    liberarHash(&ht);

    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}