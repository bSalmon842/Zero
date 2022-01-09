/*
Project: Arsene's Zero
File: zero_calculator.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

struct CalculatorState
{
    b32 isBeforeDecimal;
    s32 numbersAfterDecimalA;
    s32 numbersAfterDecimalB;
    f64 valueA;
    f64 valueB;
    f64 memValue;
    b32 valueSwitch; // NOTE(bSalmon): false = a, true = b
    char currOp;
};

enum CalcInputType
{
    CalcInput_Number,
    CalcInput_Decimal,
    CalcInput_Operator,
    CalcInput_MemOp,
};

enum MemOpType
{
    MemOp_Recall,
    MemOp_Save,
    MemOp_Clear,
};

union CalcInput
{
    s32 number;
    char op;
};

function void Calculate(CalculatorState *calcState)
{
    if (calcState->currOp == '+')
    {
        calcState->valueA += calcState->valueB;
    }
    else if (calcState->currOp == '-')
    {
        calcState->valueA -= calcState->valueB;
    }
    else if (calcState->currOp == '*')
    {
        calcState->valueA *= calcState->valueB;
    }
    else if (calcState->currOp == '/')
    {
        calcState->valueA /= calcState->valueB;
    }
    
    calcState->valueB = 0.0f;
    calcState->isBeforeDecimal = true;
    calcState->numbersAfterDecimalA = 3;
    calcState->numbersAfterDecimalB = 0;
    calcState->valueSwitch = false;
    calcState->currOp = ' ';
}

inline void ClearCalcState(CalculatorState *calcState)
{
    calcState->isBeforeDecimal = true;
    calcState->numbersAfterDecimalA = 0;
    calcState->numbersAfterDecimalB = 0;
    calcState->valueA = 0.0f;
    calcState->valueB = 0.0f;
    calcState->valueSwitch = false; // NOTE(bSalmon): false = a, true = b
    calcState->currOp = ' ';
}

function void AddInput(CalculatorState *calcState, CalcInputType inputType, CalcInput input)
{
    switch (inputType)
    {
        case CalcInput_Number:
        {
            if (calcState->isBeforeDecimal)
            {
                if (!calcState->valueSwitch)
                {
                    calcState->valueA *= 10.0f;
                    calcState->valueA += input.number;
                }
                else
                {
                    calcState->valueB *= 10.0f;
                    calcState->valueB += input.number;
                }
            }
            else
            {
                if (!calcState->valueSwitch)
                {
                    calcState->numbersAfterDecimalA++;
                    f32 decimalValue = input.number / Pow(10, calcState->numbersAfterDecimalA);
                    calcState->valueA += decimalValue;
                }
                else
                {
                    calcState->numbersAfterDecimalB++;
                    f32 decimalValue = input.number / Pow(10, calcState->numbersAfterDecimalB);
                    calcState->valueB += decimalValue;
                }
            }
        }break;
        
        case CalcInput_Decimal:
        {
            ASSERT((input.op == '.'))
                if (calcState->isBeforeDecimal)
            {
                calcState->isBeforeDecimal = false;
            }
        }break;
        
        case CalcInput_Operator:
        {
            if (input.op == '=')
            {
                if (calcState->valueSwitch)
                {
                    Calculate(calcState);
                }
            }
            else if (input.op == 'C')
            {
                ClearCalcState(calcState);
            }
            else if (input.op == '+' || input.op == '-' || input.op == '*' || input.op == '/')
            {
                if (calcState->valueSwitch)
                {
                    Calculate(calcState);
                    calcState->currOp = input.op;
                }
                else
                {
                    calcState->valueSwitch = true;
                    calcState->currOp = input.op;
                    calcState->isBeforeDecimal = true;
                }
            }
        }break;
        
        case CalcInput_MemOp:
        {
            switch (input.number)
            {
                case MemOp_Recall:
                {
                    if (!calcState->valueSwitch)
                    {
                        calcState->valueA = calcState->memValue;
                    }
                    else
                    {
                        calcState->valueB = calcState->memValue;
                    }
                }break;
                
                case MemOp_Save:
                {
                    calcState->memValue = calcState->valueA;
                }break;
                
                case MemOp_Clear:
                {
                    calcState->memValue = 0.0f;
                }break;
                
                default:
                {
                    INVALID_CODE_PATH;
                }break;
            }
        }break;
        
        default:
        {
            INVALID_CODE_PATH;
        }
    }
}

#define CalculatorNumberButton(val) if (ImGui::Button(#val, buttonSize)) { AddInput(calcState, CalcInput_Number, {val}); }
#define CalculatorDecimalButton() if (ImGui::Button(".", buttonSize)) { AddInput(calcState, CalcInput_Decimal, {'.'}); }
#define CalculatorOperatorButton(val) if (ImGui::Button(val, buttonSize)) { AddInput(calcState, CalcInput_Operator, {val[0]}); }
#define CalculatorMemOpButton(str, memOp) if (ImGui::Button(str, buttonSize)) { AddInput(calcState, CalcInput_MemOp, {memOp}); }

function void Calculator(CalculatorState *calcState)
{
    ImGui::NewLine();
    ImGui::NewLine();
    
    if (!calcState->valueSwitch)
    {
        char equation[32];
        char format[16];
        sprintf(format, "%%.0%df", calcState->numbersAfterDecimalA);
        sprintf(equation, format, calcState->valueA);
        CenteredText(equation);
    }
    else
    {
        char equation[64];
        char format[16];
        sprintf(format, "%%.0%df %c %%.0%df", calcState->numbersAfterDecimalA, calcState->currOp, calcState->numbersAfterDecimalB);
        sprintf(equation, format, calcState->valueA, calcState->valueB);
        CenteredText(equation);
    }
    
    ImVec2 buttonSize = ImVec2(ImGui::GetWindowSize().x * 0.16f, ImGui::GetWindowSize().y * 0.12f);
    
    ImGui::NewLine();
    
    CalculatorNumberButton(7);
    ImGui::SameLine();
    CalculatorNumberButton(8);
    ImGui::SameLine();
    CalculatorNumberButton(9);
    ImGui::SameLine();
    CalculatorOperatorButton("/");
    ImGui::SameLine();
    CalculatorMemOpButton("MR", MemOp_Recall);
    
    CalculatorNumberButton(4);
    ImGui::SameLine();
    CalculatorNumberButton(5);
    ImGui::SameLine();
    CalculatorNumberButton(6);
    ImGui::SameLine();
    CalculatorOperatorButton("*")
        ImGui::SameLine();
    CalculatorMemOpButton("M+", MemOp_Save);
    
    CalculatorNumberButton(1);
    ImGui::SameLine();
    CalculatorNumberButton(2);
    ImGui::SameLine();
    CalculatorNumberButton(3);
    ImGui::SameLine();
    CalculatorOperatorButton("-")
        ImGui::SameLine();
    CalculatorMemOpButton("MC", MemOp_Clear);
    
    CalculatorNumberButton(0);
    ImGui::SameLine();
    CalculatorOperatorButton("=")
        ImGui::SameLine();
    CalculatorDecimalButton()
        ImGui::SameLine();
    CalculatorOperatorButton("+");
    ImGui::SameLine();
    CalculatorOperatorButton("C");
}
