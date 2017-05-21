//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <JavaScriptCore/JSGlobalObject.h>
#include <JavaScriptCore/JSFunction.h>
#include <JavaScriptCore/SourceCode.h>
#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/Completion.h>
#include <JavaScriptCore/OpaqueJSString.h>
#include <WebCore/GCController.h>
#include <WebCore/JSDOMWindowCustom.h>
#include <WebCore/Page.h>
#include <WebCore/Frame.h>
#include <WebCore/Chrome.h>
#include <WebCore/ChromeClient.h>

#include "wkeDebug.h"

//zero ��Ҫ�󶨵�ͷ�ļ�

#include <SQLite\sqlite3.h>
#include <WinSock2.h>
#include <curl\curl.h>


#include "duv_module.h"

//cexer: ��������ں��棬��Ϊ���е� wke.h -> windows.h �ᶨ�� max��min������ WebCore �ڲ��� max��min ���ִ��ҡ�
#include "wkeWebView.h"


ZuvFuncs func;
//////////////////////////////////////////////////////////////////////////

//sqlite

zuv_ret_t duv_sqlite_open(zuv_context ctx){
	int a = func.zuv_get_top(ctx);
	if (func.zuv_is_string(ctx, 0)){
		const char *filename = func.zuv_get_string(ctx, 0);
		sqlite3 *db = NULL;
		if (sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL) != SQLITE_OK){
			//���ݿⲻ�����򴴽�
			if (sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE, NULL) == SQLITE_OK){
				//�������ݿ�
				if (func.zuv_is_string(ctx, 1)){
					int len = 0;
					const char * key = func.zuv_get_lstring(ctx, 1, &len);
					if (sqlite3_rekey(db, key, len) != SQLITE_OK){
						sqlite3_close(db);
						func.zuv_push_null(ctx);
						return 0;
					}
				}
				func.zuv_push_pointer(ctx, db);
				return 1;
			}
			//�򿪲�����ʧ��
			func.zuv_push_null(ctx);
		}
		else
		{
			//�������ݿ�
			if (func.zuv_is_string(ctx, 1)){
				int len = 0;
				const char * key = func.zuv_get_lstring(ctx, 1, &len);
				if (sqlite3_key(db, key, len) != SQLITE_OK){
					sqlite3_close(db);
					func.zuv_push_null(ctx);
					return 0;
				}
			}
			func.zuv_push_pointer(ctx, db);
		}
		return 1;
	}
	return 0;
}

static zuv_ret_t duv_sqlite_exec(zuv_context ctx){
	if (!func.zuv_is_pointer(ctx, 0) || !func.zuv_is_string(ctx, 1))
		return 0;
	sqlite3 *db = (sqlite3 *)func.zuv_get_pointer(ctx, 0);
	const char*sql = func.zuv_get_string(ctx, 1);

	sqlite3_stmt *stmt;//�����

	int ret, type;;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	//��ȡ����Ŀ
	int n_columns = sqlite3_column_count(stmt);

	int j = 0;
	zuv_idx_t arr_idx = func.zuv_push_array(ctx);	//����һ������������ŷ���ֵ
	do{
		ret = sqlite3_step(stmt);
		if (ret == SQLITE_ROW)
		{
			//����ÿһ��
			zuv_idx_t obj_idx = func.zuv_push_object(ctx);//�����ж���
			for (int i = 0; i < n_columns; i++)
			{
				/*��ȡ�д洢����*/
				type = sqlite3_column_type(stmt, i);
				switch (type)
				{
				case SQLITE_INTEGER:
					/*��������*/
					func.zuv_push_int(ctx, sqlite3_column_int(stmt, i));//�ֶ�ֵ
					break;
				case SQLITE_FLOAT:
					/*��������*/
					func.zuv_push_number(ctx, sqlite3_column_double(stmt, i));//�ֶ�ֵ
					break;
				case SQLITE_TEXT:
					/*�����ַ���*/
					func.zuv_push_string(ctx, (const char *)sqlite3_column_text(stmt, i));//�ֶ�ֵ
					break;
				case SQLITE_BLOB:{
					/*���������*/
					int len = sqlite3_column_bytes(stmt, i);
					const void *blob = sqlite3_column_blob(stmt, i);
					void *out = func.zuv_push_buffer_raw(ctx, len, 0);
					memcpy(out, blob, len);
					break;
				}
				case SQLITE_NULL:
					/*�����*/
					func.zuv_push_null(ctx);
				}
				func.zuv_put_prop_string(ctx, obj_idx, sqlite3_column_name(stmt, i));//�ֶ���
			}
			func.zuv_put_prop_index(ctx, arr_idx, j);//������������
			j++;
		}
		else //if (ret == SQLITE_DONE) //����
		{
			break;
		}
	} while (true);
	return 1;
}
static zuv_ret_t duv_sqlite_close(zuv_context ctx){
	if (!func.zuv_is_pointer(ctx, 0))
		return 0;
	sqlite3 *db = (sqlite3 *)func.zuv_get_pointer(ctx, 0);
	if (sqlite3_close(db) != SQLITE_OK){
		func.zuv_push_false(ctx);
	}
	else
	{
		func.zuv_push_true(ctx);
	}
	return 1;
}

static const zuv_function_list_entry zuv_sqlite_funcs[] = {
		{ "open", duv_sqlite_open, 2 },
		{ "exec", duv_sqlite_exec, 3 },
		{ "close", duv_sqlite_close, 1 },
		{ NULL, NULL, 0 },
};

static zuv_ret_t zuvopen_slite(zuv_context ctx) {

	func.zuv_push_global_object(ctx);
	func.zuv_put_function_list(ctx, -1, zuv_sqlite_funcs);
	func.zuv_put_prop_string(ctx, -1, "sqlite");
	return 0;
}


//curl

static const zuv_function_list_entry zuv_curl_funcs[] = {
		{ "open", duv_sqlite_open, 1 },
		{ "exec", duv_sqlite_close, 1 },
		{ "close", duv_sqlite_close, 1 },
		{ NULL, NULL, 0 },
};

static zuv_ret_t zuvopen_curl(zuv_context ctx) {

	func.zuv_push_global_object(ctx);
	func.zuv_put_function_list(ctx, -1, zuv_curl_funcs);
	func.zuv_put_prop_string(ctx, -1, "curl");
	return 0;
}

WKE_API void       WKE_CALL  ZuvModuleInit(ZuvFuncs *f){
	memcpy(&func, f, sizeof(ZuvFuncs));
	func.zuv_add_module(zuvopen_slite);
	//func.zuv_add_module(zuvopen_curl);
}
