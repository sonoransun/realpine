package com.alpine.app.data.api

import com.alpine.app.data.model.PeerListResponse
import com.alpine.app.data.model.PeerSummary
import com.alpine.app.data.model.QueryRequest
import com.alpine.app.data.model.QueryResponse
import com.alpine.app.data.model.QueryResultsResponse
import com.alpine.app.data.model.QueryStatusResponse
import com.alpine.app.data.model.ServerStatus
import retrofit2.http.Body
import retrofit2.http.DELETE
import retrofit2.http.GET
import retrofit2.http.POST
import retrofit2.http.Path

interface AlpineApiService {

    @POST("query")
    suspend fun startQuery(@Body request: QueryRequest): QueryResponse

    @GET("query/{id}")
    suspend fun getQueryStatus(@Path("id") queryId: Long): QueryStatusResponse

    @GET("query/{id}/results")
    suspend fun getQueryResults(@Path("id") queryId: Long): QueryResultsResponse

    @DELETE("query/{id}")
    suspend fun cancelQuery(@Path("id") queryId: Long)

    @GET("peers")
    suspend fun getPeers(): PeerListResponse

    @GET("peers/{id}")
    suspend fun getPeerDetail(@Path("id") peerId: Long): PeerSummary

    @GET("status")
    suspend fun getStatus(): ServerStatus
}
