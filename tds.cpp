#include "tds.h"
#include "nullable.h"
#include <sstream>
#include <iomanip>
#include <freetds/server.h>
#include <freetds/utils.h>

using namespace std;

TDSConn::TDSConn(const string& server, const string& username, const string& password, const string& app, tds_handler message_handler, tds_handler error_handler) {
	if (tds_socket_init())
		throw runtime_error("tds_socket_init failed.");

	login = tds_alloc_login(1);
	if (!login)
		throw runtime_error("tds_alloc_login failed.");

	try {
		context = tds_alloc_context(nullptr);
		if (!context)
			throw runtime_error("tds_alloc_context failed.");

		if (context->locale && !context->locale->date_fmt)
			context->locale->date_fmt = strdup(STD_DATETIME_FMT);

		try {
			if (message_handler)
				context->msg_handler = message_handler;

			if (error_handler)
				context->err_handler = error_handler;

			if (!tds_set_server(login, server.c_str()))
				throw runtime_error("tds_set_server failed.");

			if (!tds_set_user(login, username.c_str()))
				throw runtime_error("tds_set_user failed.");

			if (!tds_set_passwd(login, password.c_str()))
				throw runtime_error("tds_set_passwd failed.");

			if (!tds_set_client_charset(login, "UTF-8"))
				throw runtime_error("tds_set_client_charset failed.");

			if (!app.empty()) {
				if (!tds_set_app(login, app.c_str()))
					throw runtime_error("tds_set_app failed.");
			}

			sock = tds_alloc_socket(context, 512);
			if (!sock)
				throw runtime_error("tds_alloc_socket failed.");

			try {
				auto con = tds_read_config_info(sock, login, context->locale);
				if (!con)
					throw runtime_error("tds_read_config_info failed.");

				if (TDS_FAILED(tds_connect_and_login(sock, con))) {
					tds_free_login(con);
					throw runtime_error("tds_connect_and_login failed.");
				}

				tds_free_login(con);
			} catch (...) {
				tds_free_socket(sock);
				throw;
			}
		} catch (...) {
			tds_free_context(context);
			throw;
		}
	} catch (...) {
		tds_free_login(login);
		throw;
	}
}

TDSConn::~TDSConn() {
	if (sock)
		tds_free_socket(sock);

	if (context)
		tds_free_context(context);

	if (login)
		tds_free_login(login);
}

TDSProc::TDSProc(const TDSConn& tds, const string& q) {
	TDSRET rc;
	TDS_INT result_type;

	rc = tds_submit_query(tds.sock, q.c_str());
	if (TDS_FAILED(rc))
		throw runtime_error("tds_submit_query failed.");

	while ((rc = tds_process_tokens(tds.sock, &result_type, nullptr, TDS_TOKEN_RESULTS)) == TDS_SUCCESS) {
		const int stop_mask = TDS_STOPAT_ROWFMT | TDS_RETURN_DONE | TDS_RETURN_ROW | TDS_RETURN_COMPUTE;

		switch (result_type) {
			case TDS_COMPUTE_RESULT:
			case TDS_ROW_RESULT:
				while ((rc = tds_process_tokens(tds.sock, &result_type, nullptr, stop_mask)) == TDS_SUCCESS) {
					if (result_type != TDS_ROW_RESULT && result_type != TDS_COMPUTE_RESULT)
						break;

					if (!tds.sock->current_results)
						continue;
				}
				break;

			default:
				break;
		}
	}
}

TDSField::operator int64_t() const {
	if (null)
		return 0;
	else if (type == SYBMSDATE || type == SYBDATETIME || type == SYBDATETIMN)
		return date.dn;
	else if (type == SYBMSTIME)
		return 0;
	else if (type == SYBFLT8 || type == SYBFLTN)
		return static_cast<int64_t>(doubval);
	else if (type != SYBINTN && type != SYBBITN)
		return stoll(operator string());
	else
		return intval;
}

TDSField::operator double() const {
	if (null)
		return 0.0f;
	else if (type == SYBFLT8 || type == SYBFLTN)
		return doubval;
	else if (type == SYBMSDATE || type == SYBINTN || type == SYBBITN)
		return static_cast<double>(operator int64_t());
	else if (type == SYBMSTIME)
		return static_cast<double>((time.h * 3600) + (time.m * 60) + time.s) / 86400.0;
	else if (type == SYBDATETIME || type == SYBDATETIMN)
		return static_cast<double>(date.dn) + static_cast<double>((time.h * 3600) + (time.m * 60) + time.s) / 86400.0;
	else
		return stod(operator string());
}

TDSField::operator string() const {
	if (null)
		return "";
	else if (type == SYBINTN)
		return to_string(operator int64_t());
	else if (type == SYBMSDATE)
		return date.to_string();
	else if (type == SYBMSTIME)
		return time.to_string();
	else if (type == SYBDATETIME || type == SYBDATETIMN)
		return date.to_string() + " " + time.to_string();
	else if (type == SYBFLT8 || type == SYBFLTN)
		return to_string(doubval);
	else if (type == SYBBITN)
		return intval ? "true" : "false";
	else
		return strval;
}

static string dstr_to_string(DSTR ds) {
	return string(ds->dstr_s, ds->dstr_size);
}

static int query_message_handler(const TDSCONTEXT*, TDSSOCKET*, TDSMESSAGE* msg) {
	if ((unsigned int)msg->severity > 10)
		throw runtime_error(msg->message);

	return 0;
}

static int query_error_handler(const TDSCONTEXT*, TDSSOCKET*, TDSMESSAGE* msg) {
	throw runtime_error(msg->message);
}

void TDSQuery::start_query(const string& q) {
	TDSRET rc;
	TDS_INT result_type;

	old_msg_handler = tds.context->msg_handler;
	old_err_handler = tds.context->err_handler;

	tds.context->msg_handler = query_message_handler;
	tds.context->err_handler = query_error_handler;

	rc = tds_submit_prepare(tds.sock, q.c_str(), nullptr, &dyn, nullptr);
	if (TDS_FAILED(rc))
		throw runtime_error("tds_submit_prepare failed.");

	while ((rc = tds_process_tokens(tds.sock, &result_type, nullptr, TDS_TOKEN_RESULTS)) == TDS_SUCCESS) {
	}

	tds_free_input_params(dyn);
}

void TDSQuery::add_param2(unsigned int i, const string& param) {
	TDSPARAMINFO* params;

	params = tds_alloc_param_result(dyn->params);
	if (!params)
		throw runtime_error("tds_alloc_param_result failed.");

	dyn->params = params;
	tds_set_param_type(tds.sock->conn, params->columns[i], SYBVARCHAR);
	params->columns[i]->column_size = static_cast<TDS_INT>(param.size());
	params->columns[i]->column_cur_size = static_cast<TDS_INT>(param.size());

	tds_alloc_param_data(params->columns[i]);
	memcpy(params->columns[i]->column_data, param.c_str(), param.size());
}

void TDSQuery::add_param2(unsigned int i, int32_t v) {
	TDSPARAMINFO* params;

	params = tds_alloc_param_result(dyn->params);
	if (!params)
		throw runtime_error("tds_alloc_param_result failed.");

	dyn->params = params;
	tds_set_param_type(tds.sock->conn, params->columns[i], SYBINT4);
	params->columns[i]->column_cur_size = sizeof(v);

	tds_alloc_param_data(params->columns[i]);
	memcpy(params->columns[i]->column_data, &v, sizeof(v));
}

void TDSQuery::add_param2(unsigned int i, const TDSParam& p) {
	TDSPARAMINFO* params;

	params = tds_alloc_param_result(dyn->params);
	if (!params)
		throw runtime_error("tds_alloc_param_result failed.");

	dyn->params = params;
	tds_set_param_type(tds.sock->conn, params->columns[i], SYBVARCHAR);

	if (p.null) {
		params->columns[i]->column_size = -1;
		params->columns[i]->column_cur_size = -1;
	} else {
		params->columns[i]->column_size = static_cast<TDS_INT>(p.s.size());
		params->columns[i]->column_cur_size = static_cast<TDS_INT>(p.s.size());
	}

	tds_alloc_param_data(params->columns[i]);
	memcpy(params->columns[i]->column_data, p.s.c_str(), p.s.size());
}

void TDSQuery::end_query() {
	TDSRET rc;

	rc = tds_submit_execute(tds.sock, dyn);
	if (TDS_FAILED(rc))
		throw runtime_error("tds_submit_execute failed.");
}

bool TDSQuery::fetch_row() {
	TDSRET rc;
	TDS_INT result_type;

	while ((rc = tds_process_tokens(tds.sock, &result_type, nullptr, TDS_TOKEN_RESULTS)) == TDS_SUCCESS) {
		const int stop_mask = TDS_STOPAT_ROWFMT | TDS_RETURN_DONE | TDS_RETURN_ROW | TDS_RETURN_COMPUTE;

		switch (result_type) {
			case TDS_ROWFMT_RESULT:
				cols.clear();
				cols.reserve(tds.sock->current_results->num_cols);

				for (unsigned int i = 0; i < tds.sock->current_results->num_cols; i++) {
					string name = dstr_to_string(tds.sock->current_results->columns[i]->column_name);

					cols.emplace_back(name, tds.sock->current_results->columns[i]->column_type);
				}
				break;

			case TDS_COMPUTE_RESULT:
			case TDS_ROW_RESULT:
				if ((rc = tds_process_tokens(tds.sock, &result_type, nullptr, stop_mask)) == TDS_SUCCESS) {
					for (unsigned int i = 0; i < tds.sock->current_results->num_cols; i++) {
						auto& col = tds.sock->current_results->columns[i];

						// FIXME - other types
						cols[i].null = col->column_cur_size < 0;
						
						if (!cols[i].null) {
							if (cols[i].type == SYBVARCHAR || cols[i].type == SYBVARBINARY || cols[i].type == SYBBINARY) {		
								if (is_blob_col(col)) {
									char* s = *(char**)col->column_data;

									cols[i].strval = string(s, col->column_cur_size);
								} else
									cols[i].strval = string(reinterpret_cast<char*>(col->column_data), col->column_cur_size);
							} else if (cols[i].type == SYBINTN) {
								if (col->column_cur_size == 8) // BIGINT
									cols[i].intval = *reinterpret_cast<int64_t*>(col->column_data);
								else if (col->column_cur_size == 4) // INT
									cols[i].intval = *reinterpret_cast<int32_t*>(col->column_data);
								else if (col->column_cur_size == 2) // SMALLINT
									cols[i].intval = *reinterpret_cast<int16_t*>(col->column_data);
								else if (col->column_cur_size == 1) // TINYINT
									cols[i].intval = *reinterpret_cast<uint8_t*>(col->column_data);
							} else if (cols[i].type == SYBMSDATE) {
								auto tdta = reinterpret_cast<TDS_DATETIMEALL*>(col->column_data);

								cols[i].date = Date(tdta->date);
							} else if (cols[i].type == SYBMSTIME) {
								auto tdta = reinterpret_cast<TDS_DATETIMEALL*>(col->column_data);

								unsigned long long tm = tdta->time / 10000000;

								cols[i].time = Time(static_cast<uint8_t>(tm / 3600), (tm / 60) % 60, tm % 60);
							} else if (cols[i].type == SYBDATETIME || cols[i].type == SYBDATETIMN) {
								auto tdt = reinterpret_cast<TDS_DATETIME*>(col->column_data);

								cols[i].date = Date(tdt->dtdays);

								unsigned int tm = tdt->dttime / 300;

								cols[i].time = Time(static_cast<uint8_t>(tm / 3600), (tm / 60) % 60, tm % 60);
							} else if (cols[i].type == SYBFLTN && col->column_cur_size == 4) {
								cols[i].doubval = *reinterpret_cast<float*>(col->column_data);
							} else if (cols[i].type == SYBFLT8 || (cols[i].type == SYBFLTN && col->column_cur_size == 8)) {
								cols[i].doubval = *reinterpret_cast<double*>(col->column_data);
							} else if (cols[i].type == SYBBITN) {
								cols[i].intval = col->column_data[0] & 0x1;
							} else {
								CONV_RESULT cr;
								TDS_INT len;
								TDS_SERVER_TYPE ctype;

								ctype = tds_get_conversion_type(col->column_type, col->column_size);

								len = tds_convert(tds.sock->conn->tds_ctx, ctype, (TDS_CHAR*)col->column_data,
												  col->column_cur_size, SYBVARCHAR, &cr);

								if (len < 0)
									throw runtime_error("Failed converting type " + to_string(cols[i].type) + " to string.");

								try {
									cols[i].strval = string(cr.c, len);

									free(cr.c);
								} catch (...) {
									free(cr.c);
									throw;
								}
							}
						}
					}

					return true;
				}

			default:
				break;
		}
	}

	return false;
}

TDSField& TDSQuery::operator[](unsigned int i) {
	return cols.at(i);
}

TDSQuery::~TDSQuery() {
	TDSRET rc;
	TDS_INT result_type;

	while ((rc = tds_process_tokens(tds.sock, &result_type, nullptr, TDS_TOKEN_RESULTS)) == TDS_SUCCESS) {
		const int stop_mask = TDS_STOPAT_ROWFMT | TDS_RETURN_DONE | TDS_RETURN_ROW | TDS_RETURN_COMPUTE;

		switch (result_type) {
			case TDS_COMPUTE_RESULT:
			case TDS_ROW_RESULT:
				while ((rc = tds_process_tokens(tds.sock, &result_type, nullptr, stop_mask)) == TDS_SUCCESS) {
					if (result_type != TDS_ROW_RESULT && result_type != TDS_COMPUTE_RESULT)
						break;

					if (!tds.sock->current_results)
						continue;
				}
				break;

			default:
				break;
		}
	}

	tds_release_cur_dyn(tds.sock);

	tds.context->msg_handler = old_msg_handler;
	tds.context->err_handler = old_err_handler;
}

Date::Date(unsigned int year, unsigned int month, unsigned int day) {
	int m2 = ((int)month - 14) / 12;
	long long n;

	n = (1461 * ((int)year + 4800 + m2)) / 4;
	n += (367 * ((int)month - 2 - (12 * m2))) / 12;
	n -= (3 * (((int)year + 4900 + m2)/100)) / 4;
	n += day;
	n -= 2447096;

	dn = static_cast<int>(n);
}

void Date::calc_date() const {
	signed long long j, e, f, g, h;

	j = dn + 2415021;

	f = (4 * j) + 274277;
	f /= 146097;
	f *= 3;
	f /= 4;
	f += j;
	f += 1363;

	e = (4 * f) + 3;
	g = (e % 1461) / 4;
	h = (5 * g) + 2;

	D = ((h % 153) / 5) + 1;
	M = ((h / 153) + 2) % 12 + 1;
	Y = static_cast<unsigned int>((e / 1461) - 4716 + ((14 - M) / 12));
}

unsigned int Date::year() const {
	if (!date_calculated) {
		calc_date();
		date_calculated = true;
	}

	return Y;
}

unsigned int Date::month() const {
	if (!date_calculated) {
		calc_date();
		date_calculated = true;
	}

	return M;
}

unsigned int Date::day() const {
	if (!date_calculated) {
		calc_date();
		date_calculated = true;
	}

	return D;
}

string Date::to_string() const {
	stringstream ss;

	ss << setfill('0') << setw(4) << year();
	ss << "-";
	ss << setfill('0') << setw(2) << month();
	ss << "-";
	ss << setfill('0') << setw(2) << day();

	return ss.str();
}

string Time::to_string() const {
	stringstream ss;

	ss << setfill('0') << setw(2) << static_cast<int>(h);
	ss << ":";
	ss << setfill('0') << setw(2) << static_cast<int>(m);
	ss << ":";
	ss << setfill('0') << setw(2) << static_cast<int>(s);

	return ss.str();
}

TDSParam null_string(const string& s) {
	if (s == "")
		return TDSParam(nullptr);
	else
		return TDSParam(s);
}

TDSTrans::TDSTrans(const TDSConn& tds) : sock(tds.sock) {
	if (TDS_FAILED(tds_submit_begin_tran(sock)))
		throw std::runtime_error("tds_submit_begin_tran failed.");

	if (TDS_FAILED(tds_process_simple_query(sock)))
		throw std::runtime_error("tds_process_simple_query failed.");
}

TDSTrans::~TDSTrans() {
	if (!committed) {
		if (TDS_SUCCEED(tds_submit_rollback(sock, 0))) {
			tds_process_simple_query(sock);
		}
	}
}

void TDSTrans::commit() {
	if (TDS_FAILED(tds_submit_commit(sock, 0)))
		throw std::runtime_error("tds_submit_commit failed.");

	if (TDS_FAILED(tds_process_simple_query(sock)))
		throw std::runtime_error("tds_process_simple_query failed.");

	committed = true;
}

nullable<string>::nullable(const TDSField& f) {
	if (f.is_null())
		null = true;
	else {
		t = f;
		null = false;
	}
}

nullable<unsigned long long>::nullable(const TDSField& f) {
	if (f.is_null())
		null = true;
	else {
		t = (signed long long)f;
		null = false;
	}
}
