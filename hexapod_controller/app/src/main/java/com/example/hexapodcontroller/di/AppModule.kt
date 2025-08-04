package com.example.hexapodcontroller.di

import android.content.Context
import android.content.SharedPreferences
import com.example.hexapodcontroller.data.RobotRepositoryImpl
import com.example.hexapodcontroller.domain.RobotRepository
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import kotlinx.coroutines.Dispatchers
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object AppModule {

    @Provides
    @Singleton
    fun provideSharedPreferences(@ApplicationContext context: Context): SharedPreferences {
        return context.getSharedPreferences("hexapod_prefs", Context.MODE_PRIVATE)
    }

    @Provides
    @Singleton
    fun provideRobotRepository(
        sharedPreferences: SharedPreferences
    ): RobotRepository {
        return RobotRepositoryImpl(sharedPreferences, Dispatchers.IO)
    }
} 