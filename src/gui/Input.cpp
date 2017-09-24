#include "Input.hpp"

using namespace wp;
using namespace gui;

Input::Input(Resource& res)
{
    m_res = &res;
    m_textBuff.setFont(m_res->font);
    m_textBuff.setCharacterSize(m_res->textSize);
    m_textBuff.setFillColor(textColor);
    m_lines.setPrimitiveType(sf::Lines);
    m_lines.append(sf::Vertex(sf::Vector2f(), altBorder));
    m_lines.append(sf::Vertex(sf::Vector2f(), altBorder));
    m_lines.append(sf::Vertex(sf::Vector2f(), altBorder));
    m_lines.append(sf::Vertex(sf::Vector2f(), altBorder));
    m_lines.append(sf::Vertex(sf::Vector2f(), border));
    m_lines.append(sf::Vertex(sf::Vector2f(), border));
    m_lines.append(sf::Vertex(sf::Vector2f(), border));
    m_lines.append(sf::Vertex(sf::Vector2f(), border));

    m_lines.append(sf::Vertex(sf::Vector2f()));
    m_lines.append(sf::Vertex(sf::Vector2f()));
}
void Input::_implUpdate()
{
    if (m_password)
    {
        std::string str;
        for (int i = 0;i<m_text.size();i++)
            str += '*';
        m_textBuff.setString(str);
    }
    else
        m_textBuff.setString(m_text);
    sf::Vector2f size;
    size.x = m_textBuff.getGlobalBounds().width + m_res->textSize*0.8;
    size.y = m_res->textSize*1.8;
    if (size.x < m_minLength)
        size.x = m_minLength;
    size.x = (int)size.x;
    size.y = (int)size.y;
    m_rectangle.setSize(size);
    m_rectangle.setPosition(m_padding);
    sf::Vector2f off = sf::Vector2f(m_res->textSize*0.4, m_res->textSize*0.4) + m_padding;

    off.x = (int)off.x;
    off.y = (int)off.y;
    m_lines[0].position = sf::Vector2f(0.5, 0.5) + m_padding;
    m_lines[1].position = sf::Vector2f(size.x - 0.5, 0.5) + m_padding;
    m_lines[2].position = sf::Vector2f(0.5, 0.5) + m_padding;
    m_lines[3].position = sf::Vector2f(0.5, size.y-0.5) + m_padding;

    m_lines[4].position = sf::Vector2f(size.x - 0.5, 0.5) + m_padding;
    m_lines[5].position = sf::Vector2f(size.x - 0.5, size.y-0.5) + m_padding;
    m_lines[6].position = sf::Vector2f(0.5, size.y-0.5) + m_padding;
    m_lines[7].position = sf::Vector2f(size.x - 0.5, size.y-0.5) + m_padding;

    m_lines[8].position = sf::Vector2f(_getTextWidth(m_cursPos) - 0.5, 0.5 - m_res->textSize*0.2) + off;
    m_lines[9].position = sf::Vector2f(_getTextWidth(m_cursPos) - 0.5, m_res->textSize*1.2 - 0.5) + off;
    if ((m_internClock.getElapsedTime().asMilliseconds()%m_beamFreq.asMilliseconds()) < m_beamFreq.asMilliseconds()/2 && m_selected)
    {
        m_lines[8].color = textColor;
        m_lines[9].color = textColor;
    }
    else
    {
        m_lines[8].color = sf::Color::Transparent;
        m_lines[9].color = sf::Color::Transparent;
    }
    m_textBuff.setPosition(off);
    if (m_pressed)
    {
        m_rectangle.setFillColor(altBackground);
    }
    else
    {
        m_rectangle.setFillColor(background);
    }
}
void Input::_implEvent(sf::Event const& ev, Event& newEv, sf::View const& view)
{
    sf::Vector2f mousePos = getInverseTransform().transformPoint(m_handle->mapPixelToCoords(sf::Mouse::getPosition(*m_handle), view)) - m_padding;
    sf::Vector2f relativePos = mousePos - sf::Vector2f((int)m_res->textSize*0.4, 0);
    if (m_validClic && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        m_pressed = true;
    else
        m_pressed = false;

    if (newEv == MOUSE_LEFT_CLIC)
    {
        m_selected = true;
        m_internClock.restart();
        int min = m_text.size();
        for (int i = 0;i<m_text.size();i++)
        {
            if (_getTextWidth(i) > relativePos.x)
            {
                min = i;
                i = m_text.size();
            }
        }
        if (min > 0)
        {
            int max = min-1;
            if (std::abs(_getTextWidth(min) - relativePos.x) > std::abs(_getTextWidth(max) - relativePos.x))
                min = max;
        }
        m_cursPos = min;
        newEv += INPUT_GOT_FOCUS;
    }
    if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left && !_getHitbox().contains(mousePos))
    {
        if (m_selected)
            newEv += INPUT_LOST_FOCUS;
        m_selected = false;
    }

    if (m_selected)
    {
        if (ev.type == sf::Event::TextEntered)
        {
            char carac = ev.text.unicode;
            //std::cout << (int)carac;
            if (carac == 127);
            else if (carac == 8);   //forbidden char
            else if (carac == 22);
            else if (!m_numOnly)
            {
                m_text.insert(m_text.begin() + m_cursPos, carac);
                m_cursPos++;
                newEv += INPUT_TEXT_CHANGED;
            }
            else
            {
                if ((carac >= '0' && carac <= '9') || carac == '.' || carac == 'e' || carac == '+' || carac == '-')
                {
                    m_text.insert(m_text.begin() + m_cursPos, carac);
                    m_cursPos++;
                    newEv += INPUT_TEXT_CHANGED;
                }
            }
        }
        if (ev.type == sf::Event::KeyPressed)
        {
            if (ev.key.code == sf::Keyboard::Return)
            {
                newEv += INPUT_TEXT_RETURNED;
                newEv += INPUT_LOST_FOCUS;
                m_selected = false;
            }
            if (ev.key.code == sf::Keyboard::BackSpace)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
                {
                    std::string::iterator begin = m_text.begin() + m_cursPos;
                    char oldChar = m_text[m_cursPos-1];
                    int newPos = m_cursPos;
                    for (int i = m_cursPos-1;i>=0;i--)
                    {
                        char newChar = m_text[i];
                        if (oldChar == ' ')
                        {
                            begin = m_text.begin() + i;
                            newPos = i;
                        }
                        else
                        {
                            if (newChar != ' ')
                            {
                                begin = m_text.begin() + i;
                                newPos = i;
                            }
                            else
                            {
                                i = 0;
                            }
                        }


                        oldChar = newChar;
                    }
                    m_text.erase(begin, m_text.begin() + m_cursPos);
                    m_cursPos = newPos;
                    newEv += INPUT_TEXT_CHANGED;
                }
                else
                {
                    if (m_cursPos > 0)
                    {
                        m_text.erase(m_text.begin() + m_cursPos-1, m_text.begin() + m_cursPos);
                        m_cursPos--;
                        newEv += INPUT_TEXT_CHANGED;
                    }
                }
            }
            if (ev.key.code == sf::Keyboard::Delete)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
                {
                    std::string::iterator end = m_text.end();
                    char oldChar = m_text[m_cursPos];
                    for (int i = m_cursPos+1;i<m_text.size();i++)
                    {
                        char newChar = m_text[i];
                        if (oldChar == ' ' && newChar != ' ')
                        {
                            i = m_text.size();
                        }
                        else
                        {
                            end = m_text.begin() + i + 1;
                        }


                        oldChar = newChar;
                    }
                    m_text.erase(m_text.begin() + m_cursPos, end);
                    newEv += INPUT_TEXT_CHANGED;
                }
                else
                {
                    if (m_cursPos < m_text.size())
                    {
                        m_text.erase(m_text.begin() + m_cursPos, m_text.begin() + m_cursPos+1);
                        newEv += INPUT_TEXT_CHANGED;
                    }
                }
            }
            if (ev.key.code == sf::Keyboard::Left)
            {
                if (m_cursPos > 0)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
                    {
                        std::string::iterator end = m_text.end();
                        int newPos = 0;
                        char oldChar = m_text[m_cursPos-1];
                        for (int i = m_cursPos-1;i>=0;i--)
                        {
                            char newChar = m_text[i];
                            if (oldChar != ' ' && newChar == ' ')
                            {
                                i = 0;
                            }
                            else
                            {
                                newPos = i;
                            }


                            oldChar = newChar;
                        }
                        m_cursPos = newPos;
                        m_internClock.restart();
                    }
                    else
                    {
                        m_cursPos--;
                        m_internClock.restart();
                    }
                }
            }
            if (ev.key.code == sf::Keyboard::Right)
            {
                if (m_cursPos < m_text.size())
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
                    {
                        std::string::iterator end = m_text.end();
                        int newPos = m_text.size();
                        char oldChar = m_text[m_cursPos];
                        for (int i = m_cursPos;i<m_text.size();i++)
                        {
                            char newChar = m_text[i];
                            if (oldChar == ' ' && newChar != ' ')
                            {
                                i = m_text.size();
                            }
                            else
                            {
                                newPos = i+1;
                            }


                            oldChar = newChar;
                        }
                        m_cursPos = newPos;
                        m_internClock.restart();
                    }
                    else
                    {
                        m_cursPos++;
                        m_internClock.restart();
                    }
                }
            }
            if (ev.key.code == sf::Keyboard::V && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))
            {
                std::string str = Various::getClipboardContent();
                m_text.insert(m_text.begin() + m_cursPos, str.begin(), str.end());
                m_cursPos += str.size();
                newEv += INPUT_TEXT_CHANGED;
            }
        }
    }
    if (newEv == INPUT_TEXT_CHANGED)
        m_internClock.restart();
}
sf::FloatRect Input::_getHitbox() const
{
    sf::FloatRect rect = getLocalBounds();
    rect.left += m_padding.x;
    rect.top += m_padding.y;
    rect.width -= 2*m_padding.x;
    rect.height -= 2*m_padding.y;
    return rect;
}
sf::FloatRect Input::getLocalBounds() const
{
    sf::FloatRect rect;
    rect.width = m_rectangle.getSize().x + m_padding.x*2;
    rect.height = m_rectangle.getSize().y + m_padding.y*2;

    return rect;
}
void Input::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::View view = target.getView();
    target.setView(sf::View(sf::FloatRect(0, 0, target.getSize().x, target.getSize().y)));
    sf::Transform tr = getTransform();
    target.draw(m_rectangle, tr);
    target.draw(m_lines, tr);
    target.draw(m_textBuff, tr);
    target.setView(view);
}
void Input::setText(std::string const& text)
{
    m_text = text;
}
float Input::_getTextWidth(int nbChar) const
{
    sf::Text tmp = m_textBuff;
    std::string str = m_text.substr(0, nbChar);
    if (m_password)
    {
        for (int i = 0;i<str.size();i++)
            str[i] = '*';
    }
    tmp.setString(str);
    return tmp.getGlobalBounds().width + tmp.getLocalBounds().left;
}
void Input::setMinimumLength(float min)
{
    m_minLength = min;
}
std::string const& Input::getText() const
{
    return m_text;
}
void Input::setPassword(bool p)
{
    m_password = p;
}
void Input::setNumerical(bool n)
{
    m_numOnly = n;
}
bool Input::isSelected() const
{
    return m_selected;
}
void Input::setSelected(bool s)
{
    m_selected = s;
}
