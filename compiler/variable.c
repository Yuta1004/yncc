#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "yncc.h"

#define max(a, b) ((a)>(b) ? (a) : (b))

// 変数検索
Var *find_var(Token *request){
    // local検索(今のネスト以下の浅さで最大のものを見つける)
    Var *hit_var = NULL;
    for(int v_idx = 0; v_idx < locals->len; ++ v_idx) {
        Var *var = (Var*)vec_get(locals, v_idx);
        if( _strncmp(var->name, request->str, var->len, request->len) &&  // 名前チェック
            (vec_find(man_scope, (void*)(long)var->scope_id) != -1 || scope_id == var->scope_id) && // スコープチェック
            (hit_var == NULL || hit_var->scope_id < var->scope_id)  // 最大を残す
        ) { hit_var = var; }
    }
    if(hit_var != NULL)
        return hit_var;

    // global検索
    for(int v_idx = 0; v_idx < globals->len; ++ v_idx) {
        Var *var = (Var*)vec_get(globals, v_idx);
        if(_strncmp(var->name, request->str, var->len, request->len))
            return var;
    }

    // マッチなし
    char *name = (char*)calloc(request->len+1, sizeof(char));
    strncpy(name, request->str, request->len);
    error_at(request->str, "定義されていないか使用できない変数 : %s", name);
}

// 変数登録
Var *regist_var(int var_type){
    // 型
    Type *type = read_type();
    if(!type) return NULL;
    Type *base_type = get_base_type(type);  // 参照の先の型(base_type)

    // 変数名
    Var *var = calloc(1, sizeof(Var));
    Token *var_name = consume_ident();
    var->var_type = var_type;
    var->type = type;
    var->len = var_name->len;
    var->offset = (sum_offset += max(type->bytesize, 8));
    var->name = (char*)calloc(var_name->len+1, sizeof(char));
    var->scope_id = scope_id;
    strncpy(var->name, var_name->str, var_name->len);
    if(var_type == LOCAL)
        vec_push(locals, var);
    if(var_type == GLOBAL)
        vec_push(globals, var);

    // LOCAL -> "[" array_size "]"
    // GLOBAL -> "[" array_size? "]"
    switch(var_type){
    case LOCAL:
        if(consume("[")) {
            size_t asize = expect_number();
            copy_type(&base_type->ptr_to, base_type);
            base_type->ty = ARRAY;
            base_type->bytesize *= asize;
            var->offset = (sum_offset += base_type->bytesize - 8);
            expect("]");
            if(asize <= 0) {
                error_at(token->str, "長さが0以下の配列は定義できません");
            }
        }

    case GLOBAL:
        if(consume("[")) {
            Token *asize = consume_number();
            copy_type(&base_type->ptr_to, base_type);
            base_type->ty = ARRAY;
            if(asize)
                base_type->bytesize *= asize->val;
            expect("]");
            if(asize != 0 && asize->val <= 0) {
                error_at(token->str, "長さが0以下の配列は定義できません");
            }
        }
    }
    return var;
}