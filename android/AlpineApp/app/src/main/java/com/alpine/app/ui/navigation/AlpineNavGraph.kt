package com.alpine.app.ui.navigation

import androidx.compose.runtime.Composable
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.alpine.app.ui.screens.dashboard.DashboardScreen
import com.alpine.app.ui.screens.groups.GroupsScreen
import com.alpine.app.ui.screens.peers.PeersScreen
import com.alpine.app.ui.screens.results.ResultsScreen
import com.alpine.app.ui.screens.search.SearchScreen
import com.alpine.app.ui.screens.settings.SettingsScreen

@Composable
fun AlpineNavGraph() {
    val navController = rememberNavController()

    NavHost(navController = navController, startDestination = "dashboard") {
        composable("dashboard") {
            DashboardScreen(navController = navController)
        }
        composable("settings") {
            SettingsScreen(navController = navController)
        }
        composable("search") {
            SearchScreen(navController = navController)
        }
        composable("peers") {
            PeersScreen(navController = navController)
        }
        composable("groups") {
            GroupsScreen(navController = navController)
        }
        composable(
            route = "results/{queryId}",
            arguments = listOf(navArgument("queryId") { type = NavType.LongType })
        ) {
            ResultsScreen(navController = navController)
        }
    }
}
