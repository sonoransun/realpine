package com.alpine.app.data.transport

import com.alpine.app.data.model.QueryRequest
import com.alpine.app.data.model.QueryResponse
import com.alpine.app.data.model.QueryResultsResponse
import com.alpine.app.data.model.QueryStatusResponse
import com.alpine.app.data.rpc.AlpineRpcService

class JsonRpcTransport(private val rpcService: AlpineRpcService) : QueryTransport {

    override suspend fun startQuery(request: QueryRequest): Result<QueryResponse> {
        return try {
            val queryId = rpcService.startQuery(
                queryString = request.queryString,
                groupName = request.groupName,
                autoHaltLimit = request.autoHaltLimit,
                peerDescMax = request.peerDescMax
            )
            Result.success(QueryResponse(queryId))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    override suspend fun getQueryStatus(queryId: Long): Result<QueryStatusResponse> {
        return try {
            val inProgress = rpcService.queryInProgress(queryId)
            val status = rpcService.getQueryStatus(queryId)
            Result.success(status.copy(inProgress = inProgress))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    override suspend fun getQueryResults(queryId: Long): Result<QueryResultsResponse> {
        return try {
            Result.success(rpcService.getQueryResults(queryId))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    override suspend fun cancelQuery(queryId: Long): Result<Unit> {
        return try {
            rpcService.cancelQuery(queryId)
            Result.success(Unit)
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    override fun shutdown() {
        rpcService.shutdown()
    }
}
