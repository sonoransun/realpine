import Foundation

/// Configures retry behavior with exponential backoff and jitter for network operations.
struct RetryPolicy: Sendable {
    let maxRetries: Int
    let baseDelay: TimeInterval
    let maxDelay: TimeInterval
    let retryableStatusCodes: Set<Int>

    /// Standard retry policy: 3 retries, 1-second base delay, 30-second max.
    static let `default` = RetryPolicy(
        maxRetries: 3,
        baseDelay: 1.0,
        maxDelay: 30.0,
        retryableStatusCodes: [408, 429, 500, 502, 503, 504]
    )

    /// Aggressive retry policy: 5 retries, 0.5-second base delay, 30-second max.
    static let aggressive = RetryPolicy(
        maxRetries: 5,
        baseDelay: 0.5,
        maxDelay: 30.0,
        retryableStatusCodes: [408, 429, 500, 502, 503, 504]
    )

    /// No-retry policy.
    static let none = RetryPolicy(
        maxRetries: 0,
        baseDelay: 0,
        maxDelay: 0,
        retryableStatusCodes: []
    )

    /// Calculates delay for a given attempt using exponential backoff with random jitter.
    func delay(for attempt: Int) -> TimeInterval {
        guard attempt > 0 else { return 0 }
        let exponential = baseDelay * pow(2.0, Double(attempt - 1))
        let capped = min(exponential, maxDelay)
        let jitter = Double.random(in: 0...(capped * 0.25))
        return capped + jitter
    }

    /// Executes an async throwing operation with automatic retries on failure.
    ///
    /// Respects `Task` cancellation between retry attempts.
    func execute<T>(@_inheritActorContext _ operation: @Sendable () async throws -> T) async throws -> T {
        var lastError: Error?

        for attempt in 0...maxRetries {
            try Task.checkCancellation()

            do {
                return try await operation()
            } catch {
                lastError = error

                let shouldRetry = attempt < maxRetries && isRetryable(error)
                guard shouldRetry else { break }

                let sleepDuration = delay(for: attempt + 1)
                try await Task.sleep(nanoseconds: UInt64(sleepDuration * 1_000_000_000))
            }
        }

        throw lastError!
    }

    // MARK: - Private

    private func isRetryable(_ error: Error) -> Bool {
        if error is CancellationError { return false }

        if let urlError = error as? URLError {
            switch urlError.code {
            case .timedOut, .networkConnectionLost, .notConnectedToInternet:
                return true
            default:
                break
            }
        }

        if let apiError = error as? ApiError {
            switch apiError {
            case .httpError(let statusCode, _):
                return retryableStatusCodes.contains(statusCode)
            default:
                break
            }
        }

        return false
    }
}
