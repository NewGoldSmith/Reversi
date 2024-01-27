/**
 * @file m256_Status.h
 * @brief m_256�̃C���f�b�N�X�̐錾
 * @author Gold Smith
 * @date 2024
 * SPDX-License-Identifier: MIT
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 *
 * ���̃t�@�C�����̂��ׂẴR�[�h�́A���ɖ��L����Ă��Ȃ�����AMIT���C�Z���X�ɏ]���܂��B
 */
#pragma once
namespace ReversiEngine
{
	/// <summary>
 /// �Ֆʂ̃f�[�^��ێ�����__m256i�̓��o���p�B
 /// �T�t�B�b�N�X�̐����͎g�p����r�b�g���B
 /// OP64�̓I�v�V�����Ƃ��ė̈���������ɂ���B
 /// ST1_8�͍X�ɏ������ɂ��ăt���O�����߂�B
 /// </summary>
	const enum mIndex :unsigned {
		BB_P64, BB_O64, BB_M64, OP64, ALPHAi16 = 12, BETAi16
		, DEPTH8 = 28, NUM_TURN8, ST1_8
	};
	/// <summary>
	/// IS_MY_TURN_NOW�R���s���[�^�̎�Ԃł���B
	/// IS_C �R���s���[�^�[������(C)�Ƃ���B
	/// F1_IS_C2C�R���s���[�^�[���u�̑ΐ�B
	/// </summary>
	const enum ST1 :uint8_t { IS_MY_TURN_NOW, IS_X, IS_C2C };
}