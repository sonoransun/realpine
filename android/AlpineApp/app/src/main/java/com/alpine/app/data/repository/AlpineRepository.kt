package com.alpine.app.data.repository

import com.alpine.app.data.api.AlpineApiClient
import com.alpine.app.data.api.AlpineApiService
import com.alpine.app.data.model.PeerListResponse
import com.alpine.app.data.model.PeerSummary
import com.alpine.app.data.model.QueryRequest
import com.alpine.app.data.model.QueryResponse
import com.alpine.app.data.model.QueryResultsResponse
import com.alpine.app.data.model.QueryStatusResponse
import com.alpine.app.data.model.ServerStatus

class AlpineRepository(baseUrl: String) {

    private val service: AlpineApiService = AlpineApiClient.getService(baseUrl)

    suspend fun startQuery(request: QueryRequest): Result<QueryResponse> {
        return try {
            Result.success(service.startQuery(request))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    suspend fun getQueryStatus(queryId: Long): Result<QueryStatusResponse> {
        return try {
            Result.success(service.getQueryStatus(queryId))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    suspend fun getQueryResults(queryId: Long): Result<QueryResultsResponse> {
        return try {
            Result.success(service.getQueryResults(queryId))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    suspend fun cancelQuery(queryId: Long): Result<Unit> {
        return try {
            service.cancelQuery(queryId)
            Result.success(Unit)
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    suspend fun getPeers(): Result<PeerListResponse> {
        return try {
            Result.success(service.getPeers())
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    suspend fun getPeerDetail(peerId: Long): Result<PeerSummary> {
        return try {
            Result.success(service.getPeerDetail(peerId))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    suspend fun testConnection(): Result<ServerStatus> {
        return try {
            Result.success(service.getStatus())
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
}
