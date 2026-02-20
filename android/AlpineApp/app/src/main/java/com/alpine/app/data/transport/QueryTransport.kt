package com.alpine.app.data.transport

import com.alpine.app.data.model.QueryRequest
import com.alpine.app.data.model.QueryResponse
import com.alpine.app.data.model.QueryResultsResponse
import com.alpine.app.data.model.QueryStatusResponse

interface QueryTransport {
    suspend fun startQuery(request: QueryRequest): Result<QueryResponse>
    suspend fun getQueryStatus(queryId: Long): Result<QueryStatusResponse>
    suspend fun getQueryResults(queryId: Long): Result<QueryResultsResponse>
    suspend fun cancelQuery(queryId: Long): Result<Unit>
    fun shutdown()
}
