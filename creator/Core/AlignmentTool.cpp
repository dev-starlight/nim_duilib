#include "AlignmentTool.h"
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cstdlib>

namespace creator {

AlignmentTool::AlignmentTool(std::shared_ptr<Document> document,
                              std::shared_ptr<Selection> selection)
    : m_document(document)
    , m_selection(selection)
{
}

AlignmentTool::~AlignmentTool()
{
}

bool AlignmentTool::CanAlign() const
{
    return m_selection && m_selection->GetSelectionCount() >= 2;
}

bool AlignmentTool::CanDistribute() const
{
    return m_selection && m_selection->GetSelectionCount() >= 3;
}

bool AlignmentTool::Align(AlignType type)
{
    if (!CanAlign() || !m_document) {
        return false;
    }

    auto nodes = m_selection->GetSelectedNodes();
    if (nodes.size() < 2) {
        return false;
    }

    // 记录撤销快照
    std::string beforeXml = m_document->ToXmlString();

    // 收集所有控件的矩形
    std::vector<ui::UiRect> rects;
    for (const auto& node : nodes) {
        rects.push_back(GetControlRect(node));
    }

    switch (type) {
    case AlignType::Left: {
        // 所有控件的 left 对齐到最小值
        int32_t minLeft = rects[0].left;
        for (const auto& r : rects) {
            minLeft = std::min(minLeft, r.left);
        }
        for (size_t i = 0; i < nodes.size(); ++i) {
            SetControlPosition(nodes[i], minLeft, rects[i].top);
        }
        break;
    }
    case AlignType::Right: {
        // 所有控件的 right 边缘对齐到最大值
        int32_t maxRight = rects[0].right;
        for (const auto& r : rects) {
            maxRight = std::max(maxRight, r.right);
        }
        for (size_t i = 0; i < nodes.size(); ++i) {
            int32_t width = rects[i].Width();
            SetControlPosition(nodes[i], maxRight - width, rects[i].top);
        }
        break;
    }
    case AlignType::Top: {
        // 所有控件的 top 对齐到最小值
        int32_t minTop = rects[0].top;
        for (const auto& r : rects) {
            minTop = std::min(minTop, r.top);
        }
        for (size_t i = 0; i < nodes.size(); ++i) {
            SetControlPosition(nodes[i], rects[i].left, minTop);
        }
        break;
    }
    case AlignType::Bottom: {
        // 所有控件的 bottom 边缘对齐到最大值
        int32_t maxBottom = rects[0].bottom;
        for (const auto& r : rects) {
            maxBottom = std::max(maxBottom, r.bottom);
        }
        for (size_t i = 0; i < nodes.size(); ++i) {
            int32_t height = rects[i].Height();
            SetControlPosition(nodes[i], rects[i].left, maxBottom - height);
        }
        break;
    }
    case AlignType::CenterH: {
        // 所有控件水平中心对齐到平均值
        int32_t sumCenterX = 0;
        for (const auto& r : rects) {
            sumCenterX += r.CenterX();
        }
        int32_t avgCenterX = sumCenterX / static_cast<int32_t>(rects.size());
        for (size_t i = 0; i < nodes.size(); ++i) {
            int32_t width = rects[i].Width();
            SetControlPosition(nodes[i], avgCenterX - width / 2, rects[i].top);
        }
        break;
    }
    case AlignType::CenterV: {
        // 所有控件垂直中心对齐到平均值
        int32_t sumCenterY = 0;
        for (const auto& r : rects) {
            sumCenterY += r.CenterY();
        }
        int32_t avgCenterY = sumCenterY / static_cast<int32_t>(rects.size());
        for (size_t i = 0; i < nodes.size(); ++i) {
            int32_t height = rects[i].Height();
            SetControlPosition(nodes[i], rects[i].left, avgCenterY - height / 2);
        }
        break;
    }
    }

    // 记录撤销命令
    std::string afterXml = m_document->ToXmlString();
    auto cmd = std::make_shared<XmlSnapshotCommand>(
        "对齐控件",
        beforeXml,
        afterXml,
        [this](const std::string& xml) { m_document->LoadFromString(xml); }
    );
    m_document->GetUndoManager().RecordCommand(cmd);

    m_document->NotifyChanged(ChangeType::kAttributeChanged, m_selection->GetSelectedNode());
    return true;
}

bool AlignmentTool::Distribute(DistributeType type)
{
    if (!CanDistribute() || !m_document) {
        return false;
    }

    auto nodes = m_selection->GetSelectedNodes();
    if (nodes.size() < 3) {
        return false;
    }

    // 记录撤销快照
    std::string beforeXml = m_document->ToXmlString();

    // 收集所有控件的矩形
    std::vector<ui::UiRect> rects;
    for (const auto& node : nodes) {
        rects.push_back(GetControlRect(node));
    }

    // 创建索引数组用于排序
    std::vector<size_t> indices(nodes.size());
    std::iota(indices.begin(), indices.end(), 0);

    if (type == DistributeType::Horizontal) {
        // 按 left 排序
        std::sort(indices.begin(), indices.end(),
                  [&rects](size_t a, size_t b) { return rects[a].left < rects[b].left; });

        // 计算总宽度和总间距
        int32_t firstLeft = rects[indices.front()].left;
        int32_t lastRight = rects[indices.back()].right;
        int32_t totalWidth = 0;
        for (const auto& r : rects) {
            totalWidth += r.Width();
        }
        int32_t totalSpace = (lastRight - firstLeft) - totalWidth;
        int32_t gap = totalSpace / static_cast<int32_t>(nodes.size() - 1);

        // 从左到右分布
        int32_t currentX = firstLeft;
        for (size_t i = 0; i < indices.size(); ++i) {
            size_t idx = indices[i];
            SetControlPosition(nodes[idx], currentX, rects[idx].top);
            currentX += rects[idx].Width() + gap;
        }
    }
    else { // Vertical
        // 按 top 排序
        std::sort(indices.begin(), indices.end(),
                  [&rects](size_t a, size_t b) { return rects[a].top < rects[b].top; });

        // 计算总高度和总间距
        int32_t firstTop = rects[indices.front()].top;
        int32_t lastBottom = rects[indices.back()].bottom;
        int32_t totalHeight = 0;
        for (const auto& r : rects) {
            totalHeight += r.Height();
        }
        int32_t totalSpace = (lastBottom - firstTop) - totalHeight;
        int32_t gap = totalSpace / static_cast<int32_t>(nodes.size() - 1);

        // 从上到下分布
        int32_t currentY = firstTop;
        for (size_t i = 0; i < indices.size(); ++i) {
            size_t idx = indices[i];
            SetControlPosition(nodes[idx], rects[idx].left, currentY);
            currentY += rects[idx].Height() + gap;
        }
    }

    // 记录撤销命令
    std::string afterXml = m_document->ToXmlString();
    auto cmd = std::make_shared<XmlSnapshotCommand>(
        "分布控件",
        beforeXml,
        afterXml,
        [this](const std::string& xml) { m_document->LoadFromString(xml); }
    );
    m_document->GetUndoManager().RecordCommand(cmd);

    m_document->NotifyChanged(ChangeType::kAttributeChanged, m_selection->GetSelectedNode());
    return true;
}

ui::UiRect AlignmentTool::GetControlRect(pugi::xml_node node) const
{
    if (node.empty()) {
        return ui::UiRect();
    }

    // 从 margin 属性获取位置
    std::string marginStr = node.attribute("margin").as_string();
    ui::UiMargin margin = ParseMargin(marginStr);

    // 从 width/height 属性获取尺寸
    std::string widthStr = node.attribute("width").as_string();
    std::string heightStr = node.attribute("height").as_string();
    int32_t width = ParseDimension(widthStr, 100);
    int32_t height = ParseDimension(heightStr, 30);

    int32_t left = margin.left;
    int32_t top = margin.top;
    return ui::UiRect(left, top, left + width, top + height);
}

void AlignmentTool::SetControlPosition(pugi::xml_node node, int32_t x, int32_t y)
{
    if (node.empty()) {
        return;
    }

    // 获取当前 margin
    std::string marginStr = node.attribute("margin").as_string();
    ui::UiMargin margin = ParseMargin(marginStr);

    // 更新 left 和 top
    margin.left = x;
    margin.top = y;

    // 设置新的 margin
    std::string newMarginStr = MarginToString(margin);
    pugi::xml_attribute attr = node.attribute("margin");
    if (attr.empty()) {
        attr = node.append_attribute("margin");
    }
    attr.set_value(newMarginStr.c_str());
}

ui::UiMargin AlignmentTool::ParseMargin(const std::string& marginStr) const
{
    ui::UiMargin margin;
    if (marginStr.empty()) {
        return margin;
    }

    // 格式: "left,top,right,bottom"
    std::istringstream iss(marginStr);
    std::string token;
    int32_t values[4] = {0, 0, 0, 0};
    int32_t idx = 0;

    while (std::getline(iss, token, ',') && idx < 4) {
        values[idx++] = std::atoi(token.c_str());
    }

    margin.left = values[0];
    margin.top = values[1];
    margin.right = values[2];
    margin.bottom = values[3];
    return margin;
}

std::string AlignmentTool::MarginToString(const ui::UiMargin& margin) const
{
    std::ostringstream oss;
    oss << margin.left << "," << margin.top << ","
        << margin.right << "," << margin.bottom;
    return oss.str();
}

int32_t AlignmentTool::ParseDimension(const std::string& value, int32_t defaultVal) const
{
    if (value.empty() || value == "stretch" || value == "auto") {
        return defaultVal;
    }
    int32_t result = std::atoi(value.c_str());
    return result > 0 ? result : defaultVal;
}

} // namespace creator
