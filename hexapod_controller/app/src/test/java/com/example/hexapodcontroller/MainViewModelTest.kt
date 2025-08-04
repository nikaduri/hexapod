package com.example.hexapodcontroller

import com.example.hexapodcontroller.data.RobotCommand
import com.example.hexapodcontroller.data.RobotConnectionState
import com.example.hexapodcontroller.domain.RobotRepository
import com.example.hexapodcontroller.ui.MainViewModel
import io.mockk.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.test.*
import org.junit.After
import org.junit.Before
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class MainViewModelTest {
    private lateinit var viewModel: MainViewModel
    private lateinit var repository: RobotRepository
    private val testDispatcher = StandardTestDispatcher()

    @Before
    fun setup() {
        Dispatchers.setMain(testDispatcher)
        repository = mockk(relaxed = true)
        every { repository.connectionState } returns MutableStateFlow(RobotConnectionState.Disconnected)
        viewModel = MainViewModel(repository)
    }

    @After
    fun tearDown() {
        Dispatchers.resetMain()
    }

    @Test
    fun `when connect clicked, repository connect is called with saved settings`() = runTest {
        // Given
        val ip = "192.168.1.1"
        val port = 8080
        every { repository.getLastIpAddress() } returns ip
        every { repository.getLastPort() } returns port

        // When
        viewModel.onConnectClick()
        testDispatcher.scheduler.advanceUntilIdle()

        // Then
        coVerify { repository.connect(ip, port) }
    }

    @Test
    fun `when disconnect clicked, repository disconnect is called`() = runTest {
        // When
        viewModel.onDisconnectClick()
        testDispatcher.scheduler.advanceUntilIdle()

        // Then
        coVerify { repository.disconnect() }
    }

    @Test
    fun `when command button pressed, repository starts continuous command`() = runTest {
        // When
        viewModel.onCommandButtonPressed(RobotCommand.FORWARD)
        testDispatcher.scheduler.advanceUntilIdle()

        // Then
        coVerify { repository.startContinuousCommand(RobotCommand.FORWARD) }
    }

    @Test
    fun `when command button released, repository stops continuous command`() = runTest {
        // When
        viewModel.onCommandButtonReleased()
        testDispatcher.scheduler.advanceUntilIdle()

        // Then
        coVerify { repository.stopContinuousCommand() }
    }

    @Test
    fun `when single command sent, repository sends command once`() = runTest {
        // When
        viewModel.onSingleCommand(RobotCommand.STAND)
        testDispatcher.scheduler.advanceUntilIdle()

        // Then
        coVerify { repository.sendCommand(RobotCommand.STAND) }
    }

    @Test
    fun `when connection settings updated, repository saves settings`() {
        // Given
        val ip = "192.168.1.100"
        val port = 8081

        // When
        viewModel.updateConnectionSettings(ip, port)

        // Then
        verify { repository.saveConnectionSettings(ip, port) }
    }
} 