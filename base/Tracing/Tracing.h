/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <memory>
#include <string>


#ifdef ALPINE_TRACING_ENABLED

#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>


namespace otel_trace = opentelemetry::trace;
namespace otel_ctx = opentelemetry::context;

#endif


class Tracing
{
  public:
    static bool initialize(const string & serviceName, const string & otlpEndpoint);

    static void shutdown();

    static bool isEnabled();


#ifdef ALPINE_TRACING_ENABLED

    using t_Span = opentelemetry::nostd::shared_ptr<otel_trace::Span>;

    static t_Span startSpan(const string & name);

    static t_Span startSpan(const string & name, t_Span parentSpan);

    static string getTraceContext();

    static void setTraceContext(const string & context);

    // W3C Trace Context propagation for cross-node tracing.
    // Inject current span context into a carrier map for transmission.
    static void injectContext(std::unordered_map<string, string> & carrier);

    // Extract span context from a carrier map received from a remote node.
    // Returns the traceparent header value, or empty string if not present.
    static string extractTraceparent(const std::unordered_map<string, string> & carrier);

#endif


  private:
    static bool enabled_s;
};


class ScopedSpan
{
  public:
#ifdef ALPINE_TRACING_ENABLED

    explicit ScopedSpan(const string & name)
        : span_(Tracing::startSpan(name))
    {}

    ScopedSpan(const string & name, Tracing::t_Span parent)
        : span_(Tracing::startSpan(name, parent))
    {}

    ~ScopedSpan()
    {
        if (span_)
            span_->End();
    }

    Tracing::t_Span
    span()
    {
        return span_;
    }

    void
    setStatus(bool success)
    {
        if (span_) {
            span_->SetStatus(success ? otel_trace::StatusCode::kOk : otel_trace::StatusCode::kError);
        }
    }

    void
    addAttribute(const string & key, const string & value)
    {
        if (span_)
            span_->SetAttribute(key, value);
    }

  private:
    Tracing::t_Span span_;

#else

    explicit ScopedSpan(const string &) {}
    ScopedSpan(const string &, ...) {}
    ~ScopedSpan() = default;

    void
    setStatus(bool)
    {}
    void
    addAttribute(const string &, const string &)
    {}

#endif
};
