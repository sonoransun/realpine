package com.alpine.app.data.transport

import com.alpine.app.data.model.QueryRequest
import com.alpine.app.data.model.QueryResponse
import com.alpine.app.data.model.QueryResultsResponse
import com.alpine.app.data.model.QueryStatusResponse
import com.alpine.app.data.repository.AlpineRepository

class RestBridgeTransport(baseUrl: String) : QueryTransport {

    private val repository = AlpineRepository(baseUrl)

    override suspend fun startQuery(request: QueryRequest): Result<QueryResponse> =
        repository.startQuery(request)

    override suspend fun getQueryStatus(queryId: Long): Result<QueryStatusResponse> =
        repository.getQueryStatus(queryId)

    override suspend fun getQueryResults(queryId: Long): Result<QueryResultsResponse> =
        repository.getQueryResults(queryId)

    override suspend fun cancelQuery(queryId: Long): Result<Unit> =
        repository.cancelQuery(queryId)

    override fun shutdown() {}
}
