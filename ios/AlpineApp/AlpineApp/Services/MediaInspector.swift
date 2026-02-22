import Foundation
import ImageIO
import AVFoundation

final class MediaInspector: Sendable {

    /// Extract media metadata from a file at the given URL.
    /// Returns nil if the file is not a recognized media type or URL is nil.
    func inspect(fileURL: URL?, contentCategory: ContentCategory) -> MediaMetadata? {
        guard let fileURL else { return nil }

        switch contentCategory {
        case .image:
            return inspectImage(at: fileURL)
        case .audio:
            return inspectAudioVideo(at: fileURL, isVideo: false)
        case .video:
            return inspectAudioVideo(at: fileURL, isVideo: true)
        default:
            return nil
        }
    }

    // MARK: - Image Inspection

    private func inspectImage(at url: URL) -> MediaMetadata? {
        guard let source = CGImageSourceCreateWithURL(url as CFURL, nil) else { return nil }

        guard let properties = CGImageSourceCopyPropertiesAtIndex(source, 0, nil) as? [CFString: Any] else {
            return nil
        }

        var metadata = MediaMetadata()
        metadata.width = properties[kCGImagePropertyPixelWidth] as? Int
        metadata.height = properties[kCGImagePropertyPixelHeight] as? Int
        metadata.colorSpace = properties[kCGImagePropertyColorModel] as? String
        metadata.hasAlpha = properties[kCGImagePropertyHasAlpha] as? Bool

        return metadata
    }

    // MARK: - Audio/Video Inspection

    private func inspectAudioVideo(at url: URL, isVideo: Bool) -> MediaMetadata? {
        let asset = AVURLAsset(url: url)
        var metadata = MediaMetadata()

        // Duration
        let duration = CMTimeGetSeconds(asset.duration)
        if duration.isFinite && duration > 0 {
            metadata.durationSeconds = duration
        }

        // Video track properties
        if isVideo, let videoTrack = asset.tracks(withMediaType: .video).first {
            let size = videoTrack.naturalSize
            metadata.width = Int(size.width)
            metadata.height = Int(size.height)

            // Codec from format descriptions
            if let formatDescriptions = videoTrack.formatDescriptions as? [CMFormatDescription],
               let first = formatDescriptions.first {
                let codecType = CMFormatDescriptionGetMediaSubType(first)
                metadata.codec = fourCharCodeToString(codecType)
            }

            metadata.bitRate = Int(videoTrack.estimatedDataRate)
        }

        // Audio track properties
        if let audioTrack = asset.tracks(withMediaType: .audio).first {
            if let formatDescriptions = audioTrack.formatDescriptions as? [CMFormatDescription],
               let first = formatDescriptions.first {
                let audioStreamDesc = CMAudioFormatDescriptionGetStreamBasicDescription(first)
                if let desc = audioStreamDesc?.pointee {
                    metadata.sampleRate = Int(desc.mSampleRate)
                }
                if metadata.codec == nil {
                    let codecType = CMFormatDescriptionGetMediaSubType(first)
                    metadata.codec = fourCharCodeToString(codecType)
                }
            }
            if metadata.bitRate == nil {
                metadata.bitRate = Int(audioTrack.estimatedDataRate)
            }
        }

        // Only return if we got meaningful data
        if metadata.durationSeconds != nil || metadata.width != nil {
            return metadata
        }
        return nil
    }

    private func fourCharCodeToString(_ code: FourCharCode) -> String {
        let bytes: [CChar] = [
            CChar(truncatingIfNeeded: (code >> 24) & 0xFF),
            CChar(truncatingIfNeeded: (code >> 16) & 0xFF),
            CChar(truncatingIfNeeded: (code >> 8) & 0xFF),
            CChar(truncatingIfNeeded: code & 0xFF),
            0
        ]
        return String(cString: bytes).trimmingCharacters(in: .whitespaces)
    }
}
