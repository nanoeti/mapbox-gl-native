#include <mbgl/tile/geojson_tile.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/style/update_parameters.hpp>

#include <mapbox/geojsonvt.hpp>

namespace mbgl {

class GeoJSONTileFeature : public GeometryTileFeature {
public:
    GeoJSONTileFeature(FeatureType type_,
                       GeometryCollection&& geometries_,
                       Feature::property_map&& properties_)
        : type(type_),
          geometries(std::move(geometries_)),
          properties(std::move(properties_)) {}

    FeatureType getType() const override {
        return type;
    }

    Feature::property_map getProperties() const override {
        return properties;
    }

    GeometryCollection getGeometries() const override {
        return geometries;
    }

    optional<Value> getValue(const std::string& key) const override {
        auto it = properties.find(key);
        if (it != properties.end()) {
            return optional<Value>(it->second);
        }
        return optional<Value>();
    }

private:
    const FeatureType type;
    const GeometryCollection geometries;
    const Feature::property_map properties;
};

class GeoJSONTileLayer : public GeometryTileLayer {
public:
    std::vector<util::ptr<const GeoJSONTileFeature>> features;

    std::size_t featureCount() const override {
        return features.size();
    }

    util::ptr<const GeometryTileFeature> getFeature(std::size_t i) const override {
        return features[i];
    }

    std::string getName() const override {
        return "";
    }
};

class GeoJSONTileData : public GeometryTileData {
public:
    const util::ptr<GeoJSONTileLayer> layer = std::make_unique<GeoJSONTileLayer>();

    util::ptr<GeometryTileLayer> getLayer(const std::string&) const override {
        // We're ignoring the layer name because GeoJSON tiles only have one layer.
        return layer;
    }
};

GeoJSONTile::GeoJSONTile(const OverscaledTileID& overscaledTileID,
                         std::string sourceID,
                         const style::UpdateParameters& parameters,
                         mapbox::geojsonvt::GeoJSONVT& geojsonvt)
    : GeometryTile(overscaledTileID, sourceID, parameters.style, parameters.mode) {
    const mapbox::geojsonvt::Tile& tile = geojsonvt->getTile(id.canonical.z, id.canonical.x, id.canonical.y);
    if (!tile) {
        return;
    }

    auto data_ = std::make_unique<GeoJSONTileData>();
    auto& features = data_->layer->features;

    GeometryCoordinates line;

    for (auto& feature : tile.features) {
        const FeatureType featureType =
            (feature.type == mapbox::geojsonvt::TileFeatureType::Point
                 ? FeatureType::Point
                 : (feature.type == mapbox::geojsonvt::TileFeatureType::LineString
                        ? FeatureType::LineString
                        : (feature.type == mapbox::geojsonvt::TileFeatureType::Polygon
                               ? FeatureType::Polygon
                               : FeatureType::Unknown)));
        if (featureType == FeatureType::Unknown) {
            continue;
        }

        GeometryCollection geometry;

        // Flatten the geometry; GeoJSONVT distinguishes between a Points array and Rings array
        // (Points = GeoJSON types Point, MultiPoint, LineString)
        // (Rings = GeoJSON types MultiLineString, Polygon, MultiPolygon)
        // However, in Mapbox GL, we use one structure for both types, and just have one outer
        // element for Points.
        if (feature.tileGeometry.is<mapbox::geojsonvt::TilePoints>()) {
            line.clear();
            for (auto& point : feature.tileGeometry.get<mapbox::geojsonvt::TilePoints>()) {
                line.emplace_back(point.x, point.y);
            }
            geometry.emplace_back(std::move(line));
        } else if (feature.tileGeometry.is<mapbox::geojsonvt::TileRings>()) {
            for (auto& ring : feature.tileGeometry.get<mapbox::geojsonvt::TileRings>()) {
                line.clear();
                for (auto& point : ring) {
                    line.emplace_back(point.x, point.y);
                }
                geometry.emplace_back(std::move(line));
            }
        }

        // https://github.com/mapbox/geojson-vt-cpp/issues/44
        if (featureType == FeatureType::Polygon) {
            geometry = fixupPolygons(geometry);
        }

        Feature::property_map properties{ feature.tags.begin(), feature.tags.end() };

        features.emplace_back(std::make_shared<GeoJSONTileFeature>(
            featureType, std::move(geometry), std::move(properties)));
    }

    setData(std::move(data_));
}

} // namespace mbgl
