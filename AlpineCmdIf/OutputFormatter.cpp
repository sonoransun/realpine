/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <OutputFormatter.h>
#include <algorithm>
#include <sstream>


string
OutputFormatter::formatTable(const vector<string> & headers, const vector<vector<string>> & rows)
{
    if (headers.empty())
        return {};

    // Calculate column widths from headers and data.
    vector<size_t> widths(headers.size(), 0);

    for (size_t col = 0; col < headers.size(); ++col)
        widths[col] = headers[col].size();

    for (const auto & row : rows) {
        for (size_t col = 0; col < headers.size() && col < row.size(); ++col) {
            if (row[col].size() > widths[col])
                widths[col] = row[col].size();
        }
    }

    std::ostringstream oss;

    // Header row
    for (size_t col = 0; col < headers.size(); ++col) {
        if (col > 0)
            oss << "  ";
        oss << headers[col];
        if (col + 1 < headers.size()) {
            for (size_t p = headers[col].size(); p < widths[col]; ++p)
                oss << ' ';
        }
    }
    oss << "\n";

    // Separator
    for (size_t col = 0; col < headers.size(); ++col) {
        if (col > 0)
            oss << "  ";
        for (size_t p = 0; p < widths[col]; ++p)
            oss << '-';
    }
    oss << "\n";

    // Data rows
    for (const auto & row : rows) {
        for (size_t col = 0; col < headers.size(); ++col) {
            if (col > 0)
                oss << "  ";
            string cell = (col < row.size()) ? row[col] : ""s;
            oss << cell;
            if (col + 1 < headers.size()) {
                for (size_t p = cell.size(); p < widths[col]; ++p)
                    oss << ' ';
            }
        }
        oss << "\n";
    }

    return oss.str();
}


string
OutputFormatter::formatCsv(const vector<string> & headers, const vector<vector<string>> & rows)
{
    if (headers.empty())
        return {};

    auto quoteCsvField = [](const string & field) -> string {
        if (field.contains(',') || field.contains('"') || field.contains('\n')) {
            string escaped;
            escaped.reserve(field.size() + 2);
            escaped += '"';
            for (char c : field) {
                if (c == '"')
                    escaped += '"';
                escaped += c;
            }
            escaped += '"';
            return escaped;
        }
        return field;
    };

    std::ostringstream oss;

    // Header row
    for (size_t col = 0; col < headers.size(); ++col) {
        if (col > 0)
            oss << ',';
        oss << quoteCsvField(headers[col]);
    }
    oss << "\n";

    // Data rows
    for (const auto & row : rows) {
        for (size_t col = 0; col < headers.size(); ++col) {
            if (col > 0)
                oss << ',';
            string cell = (col < row.size()) ? row[col] : ""s;
            oss << quoteCsvField(cell);
        }
        oss << "\n";
    }

    return oss.str();
}


string
OutputFormatter::formatYaml(const vector<string> & headers, const vector<vector<string>> & rows)
{
    if (headers.empty())
        return {};

    std::ostringstream oss;

    for (const auto & row : rows) {
        oss << "- ";
        for (size_t col = 0; col < headers.size(); ++col) {
            if (col > 0)
                oss << "  ";
            string cell = (col < row.size()) ? row[col] : ""s;
            oss << headers[col] << ": " << cell << "\n";
        }
    }

    return oss.str();
}
