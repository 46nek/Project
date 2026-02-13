#pragma once
#include "MazeGenerator.h"
#include <DirectXMath.h>
#include <vector>

/**
 * @brief A*繧｢繝ｫ繧ｴ繝ｪ繧ｺ繝縺ｫ繧医ｋ邨瑚ｷｯ謗｢邏｢繧ｯ繝ｩ繧ｹ
 */
class AStar {
public:
    AStar(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData);
    ~AStar();

    std::vector<DirectX::XMFLOAT2> FindPath(int startX, int startY, int goalX, int goalY);
    bool IsWalkable(int x, int y) const;
    int GetMazeWidth() const;
    int GetMazeHeight() const;

    /**
     * @brief 繝弱・繝画ュ蝣ｱ繧剃ｿ晄戟縺吶ｋ讒矩菴・
     */
    struct NodeInfo {
        int g = 0; // 繧ｹ繧ｿ繝ｼ繝医°繧峨・遘ｻ蜍輔さ繧ｹ繝・
        int h = 0; // 繧ｴ繝ｼ繝ｫ縺ｾ縺ｧ縺ｮ謗ｨ螳壹さ繧ｹ繝・
        DirectX::XMFLOAT2 parent = { -1, -1 };
        bool inOpenList = false;
        bool inClosedList = false;

        unsigned int searchId = 0;

        int f() const { return g + h; }

        void ResetValue() {
            g = 0;
            h = 0;
            parent = { -1, -1 };  
            inOpenList = false;  
            inClosedList = false; 
        }

        void Reset() { ResetValue(); searchId = 0; }
    };

private:
    const std::vector<std::vector<MazeGenerator::CellType>>& m_mazeData;
    int m_width;
    int m_height;

    std::vector<std::vector<NodeInfo>> m_nodes;

    unsigned int m_currentSearchId = 0;
};
