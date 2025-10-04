#pragma once

class Input
{
public:
    Input();
    ~Input();

    void Initialize();
    void Frame();

    void KeyDown(unsigned int);
    void KeyUp(unsigned int);
    void MouseMove(int x, int y);

    bool IsKeyDown(unsigned int);
    void GetMouseDelta(int& x, int& y);

private:
    bool m_keys[256];
    int m_mouseX, m_mouseY;
};