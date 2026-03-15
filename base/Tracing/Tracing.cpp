/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include "Tracing.h"
#include <Log.h>

#ifdef ALPINE_TRACING_ENABLED
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>

namespace otel_sdk    = opentelemetry::sdk;
namespace otel_exp    = opentelemetry::exporter::otlp;
namespace otel_res    = opentelemetry::sdk::resource;
#endif


bool  Tracing::enabled_s = false;



bool
Tracing::initialize (const string & serviceName,
                     const string & otlpEndpoint)
{
#ifdef ALPINE_TRACING_ENABLED
    try {
        otel_exp::OtlpHttpExporterOptions opts;
        opts.url = otlpEndpoint;

        auto exporter  = otel_exp::OtlpHttpExporterFactory::Create(opts);

        otel_sdk::trace::BatchSpanProcessorOptions bspOpts;
        auto processor = otel_sdk::trace::BatchSpanProcessorFactory::Create(
            std::move(exporter), bspOpts);

        auto resource = otel_res::Resource::Create({
            {"service.name", serviceName}
        });

        auto provider = otel_sdk::trace::TracerProviderFactory::Create(
            std::move(processor), resource);

        otel_trace::Provider::SetTracerProvider(std::move(provider));

        enabled_s = true;
        Log::Info("OpenTelemetry tracing initialized: "s + otlpEndpoint);
        return true;
    } catch (const std::exception & e) {
        Log::Error("Tracing::initialize failed: "s + e.what());
        return false;
    }
#else
    Log::Info("Tracing not enabled (compiled without ALPINE_TRACING_ENABLED)"s);
    return false;
#endif
}



void
Tracing::shutdown ()
{
#ifdef ALPINE_TRACING_ENABLED
    if (enabled_s) {
        auto provider = otel_trace::Provider::GetTracerProvider();
        if (provider) {
            auto * sdkProvider = static_cast<otel_sdk::trace::TracerProvider *>(provider.get());
            if (sdkProvider)
                sdkProvider->Shutdown();
        }
        enabled_s = false;
        Log::Info("OpenTelemetry tracing shut down"s);
    }
#endif
}



bool
Tracing::isEnabled ()
{
    return enabled_s;
}



#ifdef ALPINE_TRACING_ENABLED

Tracing::t_Span
Tracing::startSpan (const string & name)
{
    auto provider = otel_trace::Provider::GetTracerProvider();
    auto tracer   = provider->GetTracer("alpine");
    return tracer->StartSpan(name);
}



Tracing::t_Span
Tracing::startSpan (const string & name,
                    t_Span         parentSpan)
{
    auto provider = otel_trace::Provider::GetTracerProvider();
    auto tracer   = provider->GetTracer("alpine");

    otel_trace::StartSpanOptions options;
    options.parent = parentSpan->GetContext();

    return tracer->StartSpan(name, options);
}



string
Tracing::getTraceContext ()
{
    // Return trace ID from current active span for propagation
    auto provider = otel_trace::Provider::GetTracerProvider();
    auto tracer   = provider->GetTracer("alpine");

    // Placeholder — in production, extract from current context
    return ""s;
}



void
Tracing::setTraceContext (const string & context)
{
    // Restore trace context from propagated header
    // Placeholder — in production, inject into current context
}


void
Tracing::injectContext (std::unordered_map<string, string> & carrier)
{
    // Inject W3C traceparent header from current active span.
    // Format: "00-<trace_id>-<span_id>-<trace_flags>"
    auto ctx = otel_ctx::RuntimeContext::GetCurrent();
    auto span = otel_trace::GetSpan(ctx);
    if (span) {
        auto spanCtx = span->GetContext();
        if (spanCtx.IsValid()) {
            char traceId[33]{};
            char spanId[17]{};
            spanCtx.trace_id().ToLowerBase16({traceId, 32});
            spanCtx.span_id().ToLowerBase16({spanId, 16});
            auto flags = spanCtx.trace_flags().IsSet(otel_trace::TraceFlags::kIsSampled) ? "01" : "00";
            carrier["traceparent"s] = std::format("00-{}-{}-{}", traceId, spanId, flags);
        }
    }
}


string
Tracing::extractTraceparent (const std::unordered_map<string, string> & carrier)
{
    auto it = carrier.find("traceparent"s);
    if (it != carrier.end()) {
        return it->second;
    }
    return ""s;
}

#endif
