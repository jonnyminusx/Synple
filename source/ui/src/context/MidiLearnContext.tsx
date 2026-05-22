import { createContext, useContext, useEffect, useState, useCallback, useRef } from 'react'
import { getNativeFunction } from '../lib/juce/index.js'

const midiLearnStart = getNativeFunction('midiLearnStart')
const midiLearnCancel = getNativeFunction('midiLearnCancel')
const midiLearnClear = getNativeFunction('midiLearnClear')
const midiLearnGetState = getNativeFunction('midiLearnGetState')

interface MidiLearnState {
  learningParam: string
  assignments: Record<string, number>
}

interface MidiLearnContextValue {
  state: MidiLearnState
  startLearn: (paramId: string) => void
  cancelLearn: () => void
  clearLearn: (paramId: string) => void
}

const defaultState: MidiLearnState = { learningParam: '', assignments: {} }

export const MidiLearnContext = createContext<MidiLearnContextValue>({
  state: defaultState,
  startLearn: () => {},
  cancelLearn: () => {},
  clearLearn: () => {},
})

export function useMidiLearnContext() {
  return useContext(MidiLearnContext)
}

export function MidiLearnProvider({ children }: { children: React.ReactNode }) {
  const [state, setState] = useState<MidiLearnState>(defaultState)
  const pollingRef = useRef<ReturnType<typeof setInterval> | null>(null)

  const poll = useCallback(async () => {
    try {
      const raw = await midiLearnGetState() as { learningParam?: string; assignments?: Record<string, number> }
      setState({
        learningParam: raw?.learningParam ?? '',
        assignments: raw?.assignments ?? {},
      })
    } catch {
      // silently ignore if not in plugin context
    }
  }, [])

  useEffect(() => {
    poll()
    pollingRef.current = setInterval(poll, 250)
    return () => {
      if (pollingRef.current !== null) clearInterval(pollingRef.current)
    }
  }, [poll])

  const startLearn = useCallback((paramId: string) => {
    midiLearnStart(paramId)
  }, [])

  const cancelLearn = useCallback(() => {
    midiLearnCancel()
  }, [])

  const clearLearn = useCallback((paramId: string) => {
    midiLearnClear(paramId)
    // optimistically update assignments so UI clears immediately
    setState(prev => {
      const next = { ...prev.assignments }
      delete next[paramId]
      return { ...prev, assignments: next }
    })
  }, [])

  return (
    <MidiLearnContext.Provider value={{ state, startLearn, cancelLearn, clearLearn }}>
      {children}
    </MidiLearnContext.Provider>
  )
}
